#include <mutex>
#include <random>
#include <utility>
#include <cstring>
#include "codec.h"
#define SKIPLIST_MAX_HEIGHT 12
template<typename K, typename V>
struct SkipNode {
    SkipNode(double score, int height, Entry<K, V>&& elem): score_(score), elem_(std::forward<Entry<K, V>>(elem)), height_(height) {
        nexts_ = new SkipNode<K, V>*[height_ + 1];
        for (int i = 0; i < height_ + 1; i++) {
            nexts_[i] = nullptr;
        }
    }
    
    SkipNode(double score, int height): score_(score), height_(height){
        nexts_ = new SkipNode<K, V>*[height_ + 1];
        for (int i = 0; i < height_ + 1; i++) {
            nexts_[i] = nullptr;
        }
    }

    ~SkipNode() {
        if (nexts_[0]) {
            delete nexts_[0];
            nexts_[0] = nullptr;
        }
        delete[] nexts_;
        nexts_ = nullptr;
    }

    void update(Entry<K, V>&& elem) {
        elem_.~Entry();
        elem_ = std::move(elem);
    }

    double score_;
    Entry<K, V> elem_;
    
    SkipNode<K, V>** nexts_;
    int height_;
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

// double CalculateKeyScore(const std::string& key) {
//     size_t len = key.size();
//     if (len > 8) {
//         len = 8;
//     }
//     uint64_t hash;
//     for (size_t i = 0; i < len; i++) {
//         uint64_t shift = uint64_t(64 - 8 - i * 8);
//         hash |= uint64_t(key[i]) << shift;
//     }
//     return double(hash);
// }

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

template<typename K, typename V>
class SkipList {
public:
    SkipList() {
        max_height_ = 0;
        header_ = new SkipNode<K, V>(-1, SKIPLIST_MAX_HEIGHT);
        prev_ = new SkipNode<K, V>*[SKIPLIST_MAX_HEIGHT];
    }

    ~SkipList() {
        if (header_->nexts_[0]) {
            delete header_->nexts_[0];
            header_->nexts_[0] = nullptr;
        }
        delete header_;
        header_ = nullptr;
        delete[] prev_;
        prev_ = nullptr;
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

    const Entry<K, V>* Contains(const K& key) const {
        double key_score = CalculateKeyScore(key);
        SkipNode<K, V>* cur_node = header_;
        int cur_height = max_height_;
        while (cur_height >= 0 && cur_node != nullptr) {
            SkipNode<K, V>* next_node = cur_node->nexts_[cur_height];
            if (next_node != nullptr && Compare(next_node, key, key_score) < 0) {
                cur_node = next_node;
            } else {
                if (cur_height == 0) {
                    if (next_node != nullptr && Compare(next_node, key, key_score) == 0) {
                        return &next_node->elem_;
                    }
                    return nullptr;
                } else {
                    cur_height--;
                }
            }
        }
        return nullptr;
    }
    

    void Insert(Entry<K, V>&& elem) {
        double key_score = CalculateScore(elem);
        // std::cout << "score:" << key_score << std::endl;
        SkipNode<K, V>* cur_node = header_;
        int cur_height = max_height_;
        while (cur_height >= 0 && cur_node != nullptr) {
            SkipNode<K, V>* next_node = cur_node->nexts_[cur_height];
            if (next_node != nullptr && Compare(next_node, elem.key_, key_score) < 0) {
                cur_node = next_node;
            } else {
                prev_[cur_height] = cur_node;
                if (cur_height == 0) {
                    if (next_node != nullptr && Compare(next_node, elem.key_, key_score) == 0) {
                        next_node->update(std::forward<Entry<K, V>>(elem));
                    }
                    break; 
                } else {
                    cur_height--;
                }
            }
        }

        int random_height = RandomHeight();

        for (int i = max_height_ + 1; i <= random_height; i++) {
            prev_[i] = header_;
        }
        max_height_ = std::max(random_height, max_height_);
        
        SkipNode<K, V>* skipnode = new SkipNode<K, V>(key_score, random_height, std::forward<Entry<K, V>>(elem));
        for (int i = 0; i <= random_height; i++) {
            skipnode->nexts_[i] = prev_[i]->nexts_[i];
        }

        for (int i = 0; i <= random_height; i++) {
            prev_[i]->nexts_[i] = skipnode;
        }
        
    }

private:
    SkipNode<K, V>* header_;
    SkipNode<K, V>** prev_;    
    int max_height_;
};
