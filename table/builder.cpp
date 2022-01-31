#include "builder.h"


Builder::Builder(const std::shared_ptr<Options>& opt): opt_(opt), key_count_(0), max_version_(0) {}

RC Builder::insert(const Entry& entry) {
    if (cur_block_ == nullptr) {
        cur_block_ = std::make_shared<Block>();
    }
    if (cur_block_->checkFinish(entry, opt_->block_size_)) {
        key_count_ += cur_block_->getKeyCount();
        cur_block_->finish();
        blocks_.push_back(cur_block_);
        cur_block_ = std::make_shared<Block>();
    }
    // cal max version
    key_hashs_.push_back(BloomFilter::hash(entry.key_.data()));
    cur_block_->insert(entry);
    return RC::SUCCESS;
}

RC Builder::flush(SSTable* sstable, uint32_t& table_crc32) {
    if (cur_block_ != nullptr) {
        key_count_ += cur_block_->getKeyCount();
        cur_block_->finish();
        blocks_.push_back(cur_block_);
    }
    int bits_per_key = BloomFilter::calBitsPerKey(key_count_, opt_->bloom_false_positive_);
    std::string filter;
    BloomFilter::createFilter(key_hashs_, filter, bits_per_key);
    std::string indexBlockContent;
    uint64_t block_len = 0;
    indexBuilder(filter, indexBlockContent, block_len);
    uint64_t index_len = indexBlockContent.size();
    uint32_t index_checksum = crc32c::Value(indexBlockContent.data(), index_len);
    table_crc32 = index_checksum;
    uint64_t total_len = block_len + index_len + sizeof(index_checksum) + 4 + 4;
    char* mmap_addr;
    RC result = sstable->bytes(0, total_len, mmap_addr);
    if (result != RC::SUCCESS) {
        filter.clear();
        indexBlockContent.clear();
        return result;
    }
    uint64_t copy_offset = SSTABLE_SIZE_LEN;
    for (size_t i = 0; i < blocks_.size(); i++) {
        std::shared_ptr<Block> block = blocks_[i];
        memmove(mmap_addr + copy_offset, block->content_.data(), block->getSize());
        copy_offset += block->getSize();
    }
    memmove(mmap_addr + copy_offset, indexBlockContent.data(), index_len);
    copy_offset += index_len;

    std::string index_checksum_str;
    encodeFix64(&index_checksum_str, index_len);
    encodeFix32(&index_checksum_str, index_checksum);
    encodeFix32(&index_checksum_str, sizeof(index_checksum));
    memmove(mmap_addr + copy_offset, index_checksum_str.data(), index_checksum_str.size());
    
    copy_offset += sizeof(index_len) + sizeof(index_checksum) + sizeof(uint32_t);
    std::string sstable_len;
    encodeFix64(&sstable_len, copy_offset);
    memmove(mmap_addr, sstable_len.data(), sstable_len.size());
    return RC::SUCCESS;
}

RC Builder::indexBuilder(const std::string& filter, std::string& content, uint64_t& block_len) {
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
        offset->set_len(block->getSize());
        block_offset += block->getSize();
    }
    block_len = block_offset;
    indexblock.SerializeToString(&content);
    return RC::SUCCESS;
}

