#ifndef CACHE_H
#define CACHE_H
#include "phmap.h"
#include "block.h"
template <typename K, typename V>
using parallel_unordered_map = phmap::parallel_flat_hash_map<K, V>;

class Cache {

public:
    static Cache& Instance() {
        static Cache cache;
        return cache;
    }
    
    
    void setBlockCache(uint64_t bId, Block&& block) {
        blockCache_.emplace(std::make_pair(bId, std::move(block)));
        // blockCache_[bId] = std::move(block);
    }

    bool getBlockCache(uint64_t bId, Block*& block) {
        if (blockCache_.find(bId) == blockCache_.end()) {
            return false;
        }
        block = &blockCache_[bId];
        return true;
        // return blockCache_[bId];
    }
private:
    parallel_unordered_map<uint64_t, Block> blockCache_;
};
#endif