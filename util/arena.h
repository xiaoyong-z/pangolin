#ifndef ARENA_H
#define ARENA_H

#include <vector>
#include <atomic>
#include "util.h"
#define BLOCKSIZE 4096

class Arena {
    Arena():cur_block_(nullptr), cur_block_offset_(0), total_memory_size_(0) {}
public:
    static Arena* Instance() {
        static Arena arena;
        return &arena;
    }

    char* allocateAlign(uint32_t size) {
        if (cur_block_ == nullptr) {
            cur_block_ = allocateBlock(BLOCKSIZE);
            cur_block_offset_ = 0;
        }
        uint32_t align_size = Util::align(size);
        if (align_size <= (BLOCKSIZE - cur_block_offset_)) {
            uint32_t old_offset = cur_block_offset_;
            cur_block_offset_ += align_size;
            return Util::align(cur_block_ + old_offset);
        } else if (align_size >= BLOCKSIZE / 4) {
            return Util::align(allocateBlock(align_size));
        } else {
            cur_block_ = allocateBlock(BLOCKSIZE);
            cur_block_offset_ = align_size;
            return Util::align(cur_block_);
        }
    }

    uint64_t getMemorySize() {
        return total_memory_size_.load(std::memory_order_relaxed);
    }

    void free() {
        for (size_t i = 0; i < blocks_.size(); i++) {
            delete[] blocks_[i];
        }
        blocks_.clear();
        cur_block_ = nullptr;
        cur_block_offset_ = 0;
        total_memory_size_.store(0, std::memory_order_relaxed);
    }

private:
    char* allocateBlock(uint32_t size) {
        char* new_block = new char[size];
        assert(new_block == Util::align(new_block));
        blocks_.push_back(new_block);
        total_memory_size_.fetch_add(size, std::memory_order_relaxed);
        return new_block;
    }

    char* cur_block_;
    uint32_t cur_block_offset_;
    std::vector<char*> blocks_;
    std::atomic<uint64_t> total_memory_size_;
};


#endif
