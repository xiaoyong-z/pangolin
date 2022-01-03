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
    // SkipNode(double score, int height, Entry&& elem): score_(score), elem_(std::forward<Entry>(elem)), height_(height) {
    //     nexts_ = new std::atomic<SkipNode*>[height_ + 1];
    //     for (int i = 0; i < height_ + 1; i++) {
    //         setNext(i, nullptr);
    //     }
    // }

    SkipNode(double score, int height): 
        key_(nullptr), key_len_(0), value_(nullptr), 
        value_len_(0), score_(score), height_(height) {
        for (int i = 0; i < height_ + 1; i++) {
            setNext(i, nullptr);
        }
    }
    
    // SkipNode(double score, int height): score_(score), height_(height){
    //     nexts_ = new std::atomic<SkipNode*>[height_ + 1];
    //     for (int i = 0; i < height_ + 1; i++) {
    //         setNext(i, nullptr);
    //     }
    // }

    ~SkipNode() {
        // if (next(0)) {
        //     delete next(0);
        //     setNext(0, nullptr);
        // }
        // delete[] nexts_;
        // nexts_ = nullptr;
    }

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

// template<typename std::string>
// double calculateKeyScore(const std::string& key) {
//     return 0;
// }

// double calculateKeyScore(const TestMove& key) {
//     return key.score_;
// }

// double calculateKeyScore(const char* key) {
//     int len = strlen(key);
//     if (len > 8) {
//         len = 8;
//     }
//     uint64_t hash = 0;
//     for (int i = 0; i < len; i++) {
//         int shift = 64 - 8 - i * 8;
//         hash |= uint64_t(key[i]) << shift;
//     }
//     return double(hash);
// }

double calculateKeyScore(const Slice& key);

// template<typename std::string>
// int compareKey(const std::string& key1, const std::string& key2) {
//     return 0;
// }

// inline int compareKey(const TestMove& key1, const TestMove& key2) {
//     return key1.a_ < key2.a_;
// }

// inline int compareKey(const char* key1, const char* key2) {
//     return strcmp(key1, key2);
// }

inline int compareKey(const Slice& key1, const Slice& key2) {
    return key1.compare(key2);
}

class SkipList;

class SkipListIterator: public Iterator<Entry, std::string> {
public:
    friend class SkipList;
    ~SkipListIterator() {};

    bool hasNext() {
        return it_ != nullptr;
    }
    void next() {
        it_ = it_->next(0);
    } 

    Entry get();

    const Entry& find(const std::string& key) {
        // Todo
        assert(false);
        // return it_->elem_;
    }

private:
    SkipNode* it_;
    SkipList* skip_list_;
};

class SkipList {
public:
    SkipList() {
        max_height_ = 0;
        arena_ = Arena::Instance();
        header_ = allocateNewNode(-1, SKIPLIST_MAX_HEIGHT, nullptr);

        // header_ = new SkipNode(-1, SKIPLIST_MAX_HEIGHT);
    }

    ~SkipList() {
        // if (header_->next(0)) {
        //     delete header_->next(0);
        //     header_->setNext(0, nullptr);
        // }
        // delete header_;
        // header_ = nullptr;
    }

    void update(SkipNode* node, Entry* elem) {
        std::lock_guard guard(update_mutex_);
        node->value_ = allocateNewValue(elem->value_, elem->expires_at_);
        node->value_len_ = elem->value_.size();
    }
    
    SkipNode* allocateNewNode(const double score, const int height, Entry* elem) {
        uint32_t size = sizeof(SkipNode) + height * sizeof(std::atomic<SkipNode*>);
        char* addr = arena_->allocateAlign(size);
        SkipNode* node = new(addr) SkipNode(score, height);
        if (elem != nullptr) {
            node->key_ = allocateNewKey(elem->key_);
            node->key_len_ = elem->key_.size();

            node->value_ = allocateNewValue(elem->value_, elem->expires_at_);
            node->value_len_ = elem->value_.size();
        }
        return node;
    }

    char* allocateNewKey(const Slice& key) {
        size_t size = key.size();
        char* addr = arena_->allocateAlign(size);
        memmove(addr, key.data(), size);
        return addr;
    }

    inline Slice getKey(char* key_data, uint32_t key_len) const{
        Slice key(key_data, key_len);
        return key; 
    }

    char* allocateNewValue(const Slice& value, const uint64_t expire_at) {
        size_t size = value.size() + sizeof(uint64_t);
        char* addr = arena_->allocateAlign(size);
        memmove(addr, value.data(), value.size());
        encodeFix64(addr + value.size(), expire_at);
        return addr;
    }

    inline ValueStruct getValue(char* value_ptr, uint32_t value_len) const{
        ValueStruct value(value_ptr, value_len, decodeFix64(value_ptr + value_len));
        return value; 
    }

    SkipListIterator* newIterator(){
        SkipListIterator* iterator = new SkipListIterator();
        iterator->it_ = header_->next(0);
        iterator->skip_list_ = this;
        return iterator;
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
        if (node->score_ > score) {
            return 1;
        } else if (node->score_ < score) {
            return -1;
        } else {
            const Slice keya = getKey(node->key_, node->key_len_);
            return compareKey(keya, keyb);
        }
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
                    if (next_node != nullptr && Compare(next_node, key, key_score) == 0) {
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
        double key_score = CalculateScore(elem->key_);
        // std::cout << "score:" << key_score << std::endl;
        SkipNode* cur_node = header_;

        int cur_height = getMaxHeight();
        while (true) {
            SkipNode* next_node = cur_node->next(cur_height);
            if (next_node != nullptr && Compare(next_node, elem->key_, key_score) < 0) {
                cur_node = next_node;
            } else {
                prev_[cur_height] = cur_node;
                if (cur_height == 0) {
                    if (next_node != nullptr && Compare(next_node, elem->key_, key_score) == 0) {
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
        return RC::SUCCESS;
    }

private:
    std::mutex update_mutex_;
    Arena* arena_;
    SkipNode* header_;
    std::atomic<int> max_height_;
};
#endif