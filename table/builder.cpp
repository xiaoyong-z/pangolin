#include "builder.h"


Builder::Builder(const std::shared_ptr<Options>& opt): opt_(opt), key_count_(0), max_version_(0), size_(0) {}

RC Builder::insert(const Entry& entry) {
    size_.fetch_add(1);
    if (cur_block_ == nullptr) {
        cur_block_ = std::make_shared<Block>(opt_->block_size_);
    }
    if (cur_block_->checkFinish(entry)) {
        key_count_ += cur_block_->getKeyCount();
        cur_block_->finish();
        blocks_.push_back(cur_block_);
        cur_block_ = std::make_shared<Block>(opt_->block_size_);
    }
    // cal max version
    key_hashs_.push_back(BloomFilter::hash(entry.getKey().data(), entry.getKey().size() - META_SIZE));
    cur_block_->insert(entry);
    return RC::SUCCESS;
}

RC Builder::flush(SSTable* sstable, uint32_t& table_crc32, bool sync) {
    if (cur_block_ != nullptr) {
        key_count_ += cur_block_->getKeyCount();
        cur_block_->finish();
        blocks_.push_back(cur_block_);
    }
    int bits_per_key = BloomFilter::calBitsPerKey(key_count_, opt_->bloom_false_positive_);
    std::string filter;
    BloomFilter::createFilter(key_hashs_, filter, bits_per_key);

    // for (size_t i = 0; i < blocks_.size(); i++) {
    //     std::unique_ptr<BlockIterator> iterator = std::make_unique<BlockIterator>(blocks_[i]);
    //     while (iterator->Valid()) {
    //         if (BloomFilter::contains(iterator->getKey().data(), filter) == false) {
    //             std::cout << "warning3: " << iterator->getKey().data() << " not in the bloom filter" << std::endl;
    //         }
    //         std::cout << "key: " << iterator->getKey() << ", value: " << iterator->getValue() << std::endl;
    //         iterator->Next();
    //     }
    // }

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
    if (sync) {
        result = sstable->sync();
        assert(result == RC::SUCCESS);
    }


    // pb::IndexBlock indexblock_;
    // indexblock_.ParseFromString(indexBlockContent);
    // for (size_t i = 0; i < blocks_.size(); i++) {
    //     std::unique_ptr<BlockIterator> iterator = std::make_unique<BlockIterator>(blocks_[i]);
    //     while (iterator->Valid()) {
    //         if (BloomFilter::contains(iterator->getKey().data(), indexblock_.bloom_filter()) == false) {
    //             std::cout << "warning2: " << iterator->getKey().data() << " not in the bloom filter" << std::endl;
    //         }
    //         std::cout << "key: " << iterator->getKey() << ", value: " << iterator->getValue() << std::endl;
    //         iterator->Next();
    //     }
    // }

    return RC::SUCCESS;
}

RC Builder::indexBuilder(const std::string& filter, std::string& content, uint64_t& block_len) {
    pb::IndexBlock indexblock;
    indexblock.set_key_count(key_count_);
    indexblock.set_max_version(max_version_);
    indexblock.set_bloom_filter(filter);

    uint64_t block_offset = 0; 
    for (size_t i = 0; i < blocks_.size(); i++) {
        std::shared_ptr<Block> block = blocks_[i];
        pb::BlockOffset* offset = indexblock.add_offsets();
        offset->set_base_key(block->base_key_);
        offset->set_offset(block_offset);
        offset->set_len(block->getSize());
        block_offset += block->getSize();
    }
    block_len = block_offset;
    indexblock.SerializeToString(&content);
    return RC::SUCCESS;
}

bool Builder::checkFinish(Entry& entry) {
    return estimateSize(entry) > opt_->SSTable_max_sz;
}

uint64_t Builder::estimateSize(Entry& entry) {
    uint64_t estimate_size = 0;
    for (size_t i = 0; i < blocks_.size(); i++) {
        // block size
        estimate_size += blocks_[i]->getSize();
        // block first key in index block + block offset + block len
        estimate_size += (blocks_[i]->base_key_.size() + 8 + 4);
    }
    if (cur_block_ != nullptr) {
        estimate_size += cur_block_->getSize();
    }
    estimate_size += BloomFilter::estimateSize(key_hashs_, key_hashs_.size(), opt_->bloom_false_positive_);
    // max_version in index block + key count in index block + index block len + checksum + checksum len 
    estimate_size += 4 + 4 + 4 + 4 + 4;
    // Todo: fix estimate_size
    estimate_size += 512;
    return estimate_size;
}

uint32_t Builder::getSize() {
    return size_.load();
}