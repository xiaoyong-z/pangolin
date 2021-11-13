#ifndef SKIPLISTS_H
#define SKIPLISTS_H

#include <mutex>
#include <random>
#include <utility>
#include <cstring>
#include <mutex>
#include "entry.h"
#include "util.h"
#define SKIPLIST_MAX_HEIGHT 12
template<typename K, typename V>
struct SkipNode {
    SkipNode(double score, int height, Entry<K, V>&& elem): score_(score), elem_(std::forward<Entry<K, V>>(elem)), height_(height) {
        nexts_ = new std::atomic<SkipNode<K, V>*>[height_ + 1];
        // nexts_ = new SkipNode<K, V>*[height_ + 1];
        for (int i = 0; i < height_ + 1; i++) {
            SetNext(i, nullptr);
        }
    }
    
    SkipNode(double score, int height): score_(score), height_(height){
        nexts_ = new std::atomic<SkipNode<K, V>*>[height_ + 1];
        // nexts_ = new SkipNode<K, V>*[height_ + 1];
        for (int i = 0; i < height_ + 1; i++) {
            SetNext(i, nullptr);
        }
    }

    ~SkipNode() {
        if (Next(0)) {
            delete Next(0);
            SetNext(0, nullptr);
        }
        delete[] nexts_;
        nexts_ = nullptr;
    }

    void update(Entry<K, V>&& elem) {
        elem_.~Entry();
        elem_ = std::move(elem);
    }

    SkipNode* Next(int n) {
        assert(n >= 0);
        // Use an 'acquire load' so that we observe a fully initialized
        // version of the returned Node.
        return nexts_[n].load(std::memory_order_acquire);
    }

    void SetNext(int n, SkipNode* x) {
        assert(n >= 0);
        // Use a 'release store' so that anybody who reads through this
        // pointer observes a fully initialized version of the inserted node.
        nexts_[n].store(x, std::memory_order_release);
    }

    SkipNode* NoBarrier_Next(int n) {
        assert(n >= 0);
        return nexts_[n].load(std::memory_order_relaxed);
    }

    void NoBarrier_SetNext(int n, SkipNode* x) {
        assert(n >= 0);
        nexts_[n].store(x, std::memory_order_relaxed);
    }

    double score_;
    Entry<K, V> elem_; 
    int height_;
private:
    std::atomic<SkipNode<K, V>*>* nexts_;
};

// template<typename K>
// double CalculateKeyScore(const K& key) {
//     return 0;
// }

double CalculateKeyScore(const TestMove& key) {
    return key.score_;
}

double CalculateKeyScore(const char* key) {
    int len = strlen(key);
    if (len > 8) {
        len = 8;
    }
    uint64_t hash = 0;
    for (int i = 0; i < len; i++) {
        int shift = 64 - 8 - i * 8;
        hash |= uint64_t(key[i]) << shift;
    }
    return double(hash);
}

double CalculateKeyScore(const std::string& key) {
    size_t len = key.size();
    if (len > 8) {
        len = 8;
    }
    uint64_t hash;
    for (size_t i = 0; i < len; i++) {
        uint64_t shift = uint64_t(64 - 8 - i * 8);
        hash |= uint64_t(key[i]) << shift;
    }
    return double(hash);
}

// template<typename K>
// int CompareKey(const K& key1, const K& key2) {
//     return 0;
// }

inline int CompareKey(const TestMove& key1, const TestMove& key2) {
    return key1.a_ < key2.a_;
}

inline int CompareKey(const char* key1, const char* key2) {
    return strcmp(key1, key2);
}

inline int CompareKey(std::string key1, std::string key2) {
    return key1.compare(key2);
}

template<typename K, typename V>
class SkipList;

template <typename K, typename V>
class SkipListIterator {
public:
    bool end() {
        return it_ != nullptr;
    }
    void next() {
        it_ = it_->Next(0);
    } 

    const Entry<K, V>& entry() {
        return it_->elem_;
    }

private:
    friend class SkipList<K, V>;
    SkipNode<K, V>* it_;
    SkipList<K, V>* skip_list_;
};

template<typename K, typename V>
class SkipList {
public:
    SkipList() {
        max_height_ = 0;
        header_ = new SkipNode<K, V>(-1, SKIPLIST_MAX_HEIGHT);
    }

    ~SkipList() {
        if (header_->Next(0)) {
            delete header_->Next(0);
            header_->SetNext(0, nullptr);
        }
        delete header_;
        header_ = nullptr;
    }

    SkipListIterator<K, V>* NewIterator(){
        SkipListIterator<K, V>* iterator = new SkipListIterator<K, V>();
        iterator->it_ = header_->Next(0);
        iterator->skip_list_ = this;
        return iterator;
    }

    int GetMaxHeight() {
        return max_height_.load(std::memory_order_relaxed);
    }


    int RandomHeight() {
        int level = 0;
        while (random() % 4 == 0)
            level += 1;
        return (level < SKIPLIST_MAX_HEIGHT) ? level : SKIPLIST_MAX_HEIGHT;
    }
    
    
    double CalculateScore(const Entry<K, V>& elem) {
        return CalculateKeyScore(elem.key_);
    }

    inline int Compare(SkipNode<K, V>* node, const K& key, double score) const {
        if (node->score_ > score) {
            return 1;
        } else if (node->score_ < score) {
            return -1;
        } else {
            return CompareKey(node->elem_.key_, (key));
        }
    }

    RC Contains(const K& key, const Entry<K, V>*& result) {
        // std::lock_guard<std::mutex> lock_guard(mutex);
        double key_score = CalculateKeyScore(key);
        SkipNode<K, V>* cur_node = header_;
        int cur_height = GetMaxHeight();
        while (true) {
            SkipNode<K, V>* next_node = cur_node->Next(cur_height);
            if (next_node != nullptr && Compare(next_node, key, key_score) < 0) {
                cur_node = next_node;
            } else {
                if (cur_height == 0) {
                    if (next_node != nullptr && Compare(next_node, key, key_score) == 0) {
                        result = &next_node->elem_;
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
    

    RC Insert(Entry<K, V>&& elem) {
        SkipNode<K, V>* prev_[SKIPLIST_MAX_HEIGHT];
        double key_score = CalculateScore(elem);
        // std::cout << "score:" << key_score << std::endl;
        SkipNode<K, V>* cur_node = header_;

        int cur_height = GetMaxHeight();
        while (true) {
            SkipNode<K, V>* next_node = cur_node->Next(cur_height);
            if (next_node != nullptr && Compare(next_node, elem.key_, key_score) < 0) {
                cur_node = next_node;
            } else {
                prev_[cur_height] = cur_node;
                if (cur_height == 0) {
                    if (next_node != nullptr && Compare(next_node, elem.key_, key_score) == 0) {
                        next_node->update(std::forward<Entry<K, V>>(elem));
                        return RC::SUCCESS;
                    }
                    break; 
                } else {
                    cur_height--;
                }
            }
        }

        int random_height = RandomHeight();

        if (random_height > GetMaxHeight()) {
            for (int i = GetMaxHeight() + 1; i <= random_height; i++) {
                prev_[i] = header_;
            }
            max_height_.store(random_height, std::memory_order_relaxed);
        }

        SkipNode<K, V>* skip_node = new SkipNode<K, V>(key_score, random_height, std::forward<Entry<K, V>>(elem));
        for (int i = 0; i <= random_height; i++) {
            skip_node->NoBarrier_SetNext(i, prev_[i]->NoBarrier_Next(i));
            prev_[i]->SetNext(i, skip_node);
        }
        return RC::SUCCESS;
    }

private:
    SkipNode<K, V>* header_;
    std::atomic<int> max_height_;
};
using STRSkipList = SkipList<std::string, std::string>;

using STRSkipListIterator = SkipListIterator<std::string, std::string>;
#endif