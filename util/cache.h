#ifndef CACHE_H
#define CACHE_H
#include "block.h"

class Block;

class Cache {

public:
    static Cache& Instance() {
        static Cache cache;
        return cache;
    }
    
    
    void setBlockCache(uint64_t bId, Block&& block) {
        // unfinished
        assert (false);
    }

    bool getBlockCache(uint64_t bId, Block*& block) {
        // unfinished
        assert (false);
        
        return true;
    }
private:
};
#endif