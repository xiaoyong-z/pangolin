#include <mutex>
#include <random>
#include <utility>
#include "codec.h"
#define MAX_LEVEL 64

template<typename K, typename V>
struct SkipNode {
    double score_;
    Entry<K, V>* elem_;
    SkipNode* nexts_;
};

template<typename K, typename V>
class SkipList {
public:
    SkipList() {

    }
    
    
    double calculateScore(Entry<K, V>&& elem) {
        return 1;
    }


    const Entry<K, V>*& contains(const K key) const {
        double key_score = 1;
        SkipNode<K, V>* cur_node = header_;
        int cur_height = max_level_;
        while (cur_height >= 0 && header_ != nullptr) {
            SkipNode<K, V>* next_node = cur_node->nexts_[cur_height]->next;
            if (cur_node->score_ < key_score) {
                cur_node = next_node;
            } else {
                if (cur_height == 0) {
                    if (next_node->score == key_score) {
                        return next_node;
                    }
                    return nullptr;
                } else {
                    cur_height--;
                }
            }
        }
    }
    

    void insert(Entry<K, V>&& elem) {
        double key_score = calculateScore(std::forward(elem));
        // SkipNode<K, V>* cur_node = header_;
        // int cur_height = max_level_;
        // while (cur_height >= 0 && header_ != nullptr) {
        //     SkipNode<K, V>* next_node = cur_node->nexts_[cur_height]->next;
        //     if (cur_node->score_ < key_score) {
        //         cur_node = next_node;
        //     } else {
        //         if (cur_height == 0) {
        //             if (next_node->score == key_score) {
        //                 return;
        //             }
        //             break;
        //         } else {
        //             cur_height--;
        //         }
        //     }
        // }
    }

private:
    SkipNode<K, V>* header_;
    SkipNode<K, V>** prev_;    
    int max_level_;
};
