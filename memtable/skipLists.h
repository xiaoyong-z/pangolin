#ifndef SKIPLISTS_H
#define SKIPLISTS_H

#include <mutex>
#include <random>
#include <utility>
#include <cstring>
#include <mutex>
#include "entry.h"
#include "util.h"
#include "iterator.h"
#include "arena.h"
#define SKIPLIST_MAX_HEIGHT 12
struct SkipNode {
    SkipNode(double score, int height): 
        key_(nullptr), key_len_(0), value_(nullptr), 
        value_len_(0), score_(score), height_(height) {
        for (int i = 0; i < height_ + 1; i++) {
            setNext(i, nullptr);
        }
    }

    ~SkipNode() {}

    SkipNode* next(int n) {
        assert(n >= 0);
        // Use an 'acquire load' so that we observe a fully initialized
        // version of the returned Node.
        return nexts_[n].load(std::memory_order_acquire);
    }

    void setNext(int n, SkipNode* x) {
        assert(n >= 0);
        // Use a 'release store' so that anybody who reads through this
        // pointer observes a fully initialized version of the inserted node.
        nexts_[n].store(x, std::memory_order_release);
    }

    SkipNode* noBarrier_Next(int n) {
        assert(n >= 0);
        return nexts_[n].load(std::memory_order_relaxed);
    }

    void noBarrier_SetNext(int n, SkipNode* x) {
        assert(n >= 0);
        nexts_[n].store(x, std::memory_order_relaxed);
    }

    char* key_;
    uint32_t key_len_;
    char* value_;
    uint32_t value_len_;

    double score_;
    int height_;
private:
    std::atomic<SkipNode*> nexts_[1];
};

double calculateKeyScore(const Slice& key);

class SkipList {
public:
    SkipList() {
        max_height_ = 0;
        arena_ = Arena::Instance();
        header_ = allocateNewNode(-1, SKIPLIST_MAX_HEIGHT, nullptr);
        entry_count = 0;
    }

    ~SkipList() {}

    SkipNode* firstNode() {
        return header_->next(0);
    }

    void update(SkipNode* node, Entry* elem) {
        std::lock_guard guard(update_mutex_);
        node->value_ = allocateNewValue(elem->getValue());
        node->value_len_ = elem->getValue().size();
    }
    
    SkipNode* allocateNewNode(const double score, const int height, Entry* elem) {
        uint32_t size = sizeof(SkipNode) + height * sizeof(std::atomic<SkipNode*>);
        char* addr = arena_->allocateAlign(size);
        SkipNode* node = new(addr) SkipNode(score, height);
        if (elem != nullptr) {
            node->key_ = allocateNewKey(elem->getKey());
            node->key_len_ = elem->getKey().size();

            node->value_ = allocateNewValue(elem->getValue());
            node->value_len_ = elem->getValue().size();
        }
        return node;
    }

    char* allocateNewKey(const Slice& key) {
        size_t size = key.size() + 1;
        char* addr = arena_->allocateAlign(size);
        memmove(addr, key.data(), size);
        addr[size] = '\0';
        return addr;
    }

    inline Slice getKey(char* key_data, uint32_t key_len) const{
        Slice key(key_data, key_len);
        return key; 
    }

    char* allocateNewValue(const Slice& value) {
        size_t size = value.size() + 1;
        char* addr = arena_->allocateAlign(size);
        memmove(addr, value.data(), value.size());
        addr[size] = '\0';
        return addr;
    }

    inline ValueStruct getValue(char* value_ptr, uint32_t value_len) const{
        ValueStruct value(value_ptr, value_len);
        return value; 
    }

    int getMaxHeight() {
        return max_height_.load(std::memory_order_relaxed);
    }

    int randomHeight() {
        int level = 0;
        while (random() % 4 == 0)
            level += 1;
        return (level < SKIPLIST_MAX_HEIGHT) ? level : SKIPLIST_MAX_HEIGHT;
    }
    
    
    double CalculateScore(const Slice& key) {
        return calculateKeyScore(key);
    }

    inline int Compare(SkipNode* node, const Slice& keyb, double score) const {
        // if (node->score_ > score) {
        //     return 1;
        // } else if (node->score_ < score) {
        //     return -1;
        // } else {
        const Slice keya = getKey(node->key_, node->key_len_);
        return Util::compareBytes(keya, keyb);
        // }
    }

    inline int KeyCompare(SkipNode* node, const Slice& keyb, double score) const {
        const Slice keya = getKey(node->key_, node->key_len_);
        int cmp = Util::compareKey(keya, keyb);
        return cmp;
    }

    RC contains(const Slice& key, Entry& result) {
        // std::lock_guard<std::mutex> lock_guard(mutex);
        double key_score = calculateKeyScore(key);
        SkipNode* cur_node = header_;
        int cur_height = getMaxHeight();
        while (true) {
            SkipNode* next_node = cur_node->next(cur_height);
            if (next_node != nullptr && Compare(next_node, key, key_score) < 0) {
                cur_node = next_node;
            } else {
                if (cur_height == 0) {
                    if (next_node != nullptr && KeyCompare(next_node, key, key_score) == 0) {
                        const Slice key_find = getKey(next_node->key_, next_node->key_len_);
                        const ValueStruct value_find = getValue(next_node->value_, next_node->value_len_);
                        result.reset(key_find, value_find);
                        return RC::SUCCESS;
                    }
                    return RC::SKIPLIST_NOT_FOUND;
                } else {
                    cur_height--;
                }
            }
        }
        return RC::SKIPLIST_NOT_FOUND;
    }
    

    RC insert(Entry* elem) {
        SkipNode* prev_[SKIPLIST_MAX_HEIGHT];
        double key_score = CalculateScore(elem->getKey());
        // std::cout << "score:" << key_score << std::endl;
        SkipNode* cur_node = header_;

        int cur_height = getMaxHeight();
        while (true) {
            SkipNode* next_node = cur_node->next(cur_height);
            if (next_node != nullptr && Compare(next_node, elem->getKey(), key_score) < 0) {
                cur_node = next_node;
            } else {
                prev_[cur_height] = cur_node;
                if (cur_height == 0) {
                    if (next_node != nullptr && Compare(next_node, elem->getKey(), key_score) == 0) {
                        update(next_node, elem);
                        return RC::SUCCESS;
                    }
                    break; 
                } else {
                    cur_height--;
                }
            }
        }

        int random_height = randomHeight();

        if (random_height > getMaxHeight()) {
            for (int i = getMaxHeight() + 1; i <= random_height; i++) {
                prev_[i] = header_;
            }
            max_height_.store(random_height, std::memory_order_relaxed);
        }

        SkipNode* skip_node = allocateNewNode(key_score, random_height, elem);
        for (int i = 0; i <= random_height; i++) {
            skip_node->noBarrier_SetNext(i, prev_[i]->noBarrier_Next(i));
            prev_[i]->setNext(i, skip_node);
        }
        entry_count++;
        return RC::SUCCESS;
    }

    int getEntryCount() {
        return entry_count;
    }

private:
    std::mutex update_mutex_;
    Arena* arena_;
    SkipNode* header_;
    std::atomic<int> max_height_;
    int entry_count;
};
#endif