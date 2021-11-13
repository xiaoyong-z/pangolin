#ifndef BUILDER_H
#define BUILDER_H
#include <vector>
#include "file.h"
#include "block.h"
#include "util.h"
#include "bloomFilter.h"
#include "indexBlock.h"
class Builder {
public:
    Builder(const std::shared_ptr<Options>& opt): opt_(opt), key_count_(0), max_version_(0) {}

    RC insert(const strEntry& entry) {
        if (cur_block_ == nullptr) {
            cur_block_ = std::make_shared<Block>();
        }
        if (cur_block_->checkFinish(entry, opt_->block_size_)) {
            key_count_ += cur_block_->GetKeyCount();
            cur_block_->Finish();
            blocks_.push_back(cur_block_);
            cur_block_ = std::make_shared<Block>();
        }
        // cal max version
        key_hashs_.push_back(BloomFilter::Hash(entry.key_.data()));
        cur_block_->insert(entry);
        return RC::SUCCESS;
    }

    RC done() {
        if (cur_block_ != nullptr) {
            key_count_ += cur_block_->GetKeyCount();
            cur_block_->Finish();
            blocks_.push_back(cur_block_);
        }
        int bits_per_key = BloomFilter::calBitsPerKey(key_count_, opt_->bloom_false_positive_);
        std::string filter;
        BloomFilter::createFilter(key_hashs_, filter, bits_per_key);
        IndexBuilder(filter);
    }

    RC IndexBuilder(const std::string& filter) {
        IndexBlock indexblock;
        indexblock.key_count_ = key_count_;
        indexblock.max_version_ = max_version_;
        indexblock.bloom_filter_ = std::move(filter);
        uint64_t block_offset = 0;
        for (size_t i = 0; i < blocks_.size(); i++) {
            std::shared_ptr<Block> block = blocks_[i];
            indexblock.offsets_.emplace_back(std::move(block->base_key_), block_offset, block->current_offset);
            block_offset += block->current_offset;
        }
        return RC::SUCCESS;
    }


private:
    std::shared_ptr<Options> opt_;
    std::shared_ptr<Block> cur_block_;
    std::vector<std::shared_ptr<Block>> blocks_;
    std::vector<uint32_t> key_hashs_;
    uint32_t key_count_;
    uint32_t max_version_;
};
#endif
