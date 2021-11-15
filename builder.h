#ifndef BUILDER_H
#define BUILDER_H
#include <vector>
#include "file.h"
#include "block.h"
#include "util.h"
#include "bloomFilter.h"
#include "sstable.h"
#include "kv.pb.h"
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

    RC flush(SSTable* sstable) {
        
        if (cur_block_ != nullptr) {
            key_count_ += cur_block_->GetKeyCount();
            cur_block_->Finish();
            blocks_.push_back(cur_block_);
        }
        int bits_per_key = BloomFilter::calBitsPerKey(key_count_, opt_->bloom_false_positive_);
        std::string filter;
        BloomFilter::createFilter(key_hashs_, filter, bits_per_key);
        std::string indexBlockContent;
        uint64_t block_len = 0;
        IndexBuilder(filter, indexBlockContent, block_len);
        uint64_t index_len = indexBlockContent.size();
        uint32_t index_checksum = crc32c::Value(indexBlockContent.data(), index_len);
        uint64_t total_len = block_len + index_len + sizeof(index_checksum) + 4 + 4;
        char* mmap_addr;
        RC result = sstable->Bytes(0, total_len, mmap_addr);
        if (result != RC::SUCCESS) {
            filter.clear();
            indexBlockContent.clear();
            return result;
        }
        uint64_t copy_offset = 0;
        for (size_t i = 0; i < blocks_.size(); i++) {
            std::shared_ptr<Block> block = blocks_[i];
            memcpy(mmap_addr + copy_offset, block->content.data(), block->current_offset);
            copy_offset += block->current_offset;
        }
        memcpy(mmap_addr + copy_offset, indexBlockContent.data(), index_len);
        copy_offset += index_len;

        std::string index_checksum_str;
        EncodeFix64(&index_checksum_str, index_len);
        EncodeFix32(&index_checksum_str, index_checksum);
        EncodeFix32(&index_checksum_str, 4);
        memcpy(mmap_addr + copy_offset, index_checksum_str.data(), index_checksum_str.size());

        return RC::SUCCESS;
    }

    RC IndexBuilder(const std::string& filter, std::string& content, uint64_t& block_len) {
        pb::IndexBlock indexblock;
        indexblock.set_key_count(key_count_);
        indexblock.set_max_version(max_version_);
        indexblock.set_bloom_filter(std::move(filter));

        uint64_t block_offset = 0; 
        for (size_t i = 0; i < blocks_.size(); i++) {
            std::shared_ptr<Block> block = blocks_[i];
            pb::BlockOffset* offset = indexblock.add_offsets();
            offset->set_base_key(std::move(block->base_key_));
            offset->set_offset(block_offset);
            offset->set_len(block->current_offset);
            block_offset += block->current_offset;
        }
        block_len = block_offset;
        indexblock.SerializeToString(&content);
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