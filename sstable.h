#ifndef SSTABLE_FILE_H
#define SSTABLE_FILE_H

#include "file.h"
#include "util.h"
#include "kv.pb.h"
#include "iterator.h"
class BlockOffsetsIterator{
    BlockOffsetsIterator(uint32_t block_offset_index, const pb::IndexBlock& indexblock): 
        max_offset_index_(block_offset_index), cur_offset_index_(0), indexblock_(indexblock) {  }

    friend class SSTable;
public: 
    bool hasNext() {
        return cur_offset_index_ != max_offset_index_;
    }

    void next() {
        cur_offset_index_++;
    }

    const uint32_t get() {
        return cur_offset_index_;
    }

    const uint32_t find(const std::string& key) {
        uint32_t min = 0, max = max_offset_index_ - 1;
        while(min < max) {
            uint32_t mid = min + (max - min) / 2 + 1;
            assert(mid >= 0 && mid <= max_offset_index_ - 1);
            const pb::BlockOffset& blockOffset = indexblock_.offsets(mid);
            const std::string& base_key= blockOffset.base_key();
            if (base_key == key) {
                return mid;
            } else if (base_key < key) {
                min = mid;
            } else {
                max = mid - 1;
            }
        }
        return min;
    }

    const pb::BlockOffset& getBlockOffset(uint32_t index) {
        return indexblock_.offsets(index);
    }

private:
    uint32_t max_offset_index_;
    uint32_t cur_offset_index_;
    const pb::IndexBlock& indexblock_;
    
};
class SSTable {
    friend class Table;
    friend class Block;
private:
    SSTable(MmapFile* mmap_file): file_(mmap_file){}
public:
    static SSTable* newSSTableFile(const std::shared_ptr<FileOptions>& opt) {
        MmapFile* mmap_file = MmapFile::newMmapFile(opt->file_name_, opt->flag_, opt->max_sz_);
        if (mmap_file == nullptr) {
            return nullptr;
        }
        SSTable* sstable = new SSTable(mmap_file);
        return sstable;
    }

    RC bytes(uint64_t offset, uint64_t size, char*& mmap_addr) {
        return file_->bytes(offset, size, mmap_addr);
    }
    
    RC init() {
        char* mmap_ptr;
        RC result = file_->get_mmap_ptr(mmap_ptr);
        if (result != RC::SUCCESS) {
            return result;
        }
        raw_ptr_ = mmap_ptr;
        uint64_t sstable_len = decodeFix64(mmap_ptr);
        mmap_ptr += sstable_len;
        uint32_t check_sum_len = decodeFix32(mmap_ptr - sizeof(uint32_t));
        mmap_ptr -= sizeof(uint32_t);
        uint32_t check_sum = decodeFix32(mmap_ptr - check_sum_len);
        mmap_ptr -= check_sum_len;
        uint64_t index_len = decodeFix64(mmap_ptr - sizeof(uint64_t));
        mmap_ptr -= sizeof(uint64_t);

        std::string index(mmap_ptr - index_len, index_len);       
        uint32_t index_checksum = crc32c::Value(index.data(), index_len);
        if (index_checksum != check_sum) {
            return RC::SSTABLE_CRC_FAIL;
        }
        indexblock_.ParseFromString(index);
        mmap_ptr -= index_len;
        bloom_filter_.reset(indexblock_.release_bloom_filter());
        
        block_offset_index_ = indexblock_.offsets_size();
        assert(block_offset_index_ > 0);

        min_key_ = indexblock_.offsets(0).base_key();


        std::string max_block_base_key = indexblock_.offsets(block_offset_index_ - 1).base_key();
        uint64_t max_block_offset = indexblock_.offsets(block_offset_index_ - 1).offset();
        uint64_t max_block_len = indexblock_.offsets(block_offset_index_ - 1).len();

        std::string block_content(raw_ptr_ + SSTABLE_SIZE_LEN + max_block_offset, max_block_len);
        
        uint32_t block_check_sum_len = decodeFix32(block_content.data() + block_content.size() - 4);
        uint32_t block_check_sum = decodeFix32(block_content.data() + block_content.size() - 4 - block_check_sum_len);
        uint32_t block_crc_value = crc32c::Value(block_content.data(), block_content.size() - 4 - block_check_sum_len);
        if (block_crc_value != block_check_sum) {
            return RC::SSTABLE_CRC_FAIL;
        }
        uint32_t last_offset = decodeFix32(block_content.data() + block_content.size() - 4 - block_check_sum_len - 8);
        uint32_t header = decodeFix32(block_content.data() + last_offset);
        uint16_t overlap = header;
        uint16_t diff = header >> 16;
        std::string diff_key(block_content.data() + last_offset + 4, diff);
        max_key_ = max_block_base_key.substr(0, overlap) + diff_key;

        return RC::SUCCESS;
    }

    BlockOffsetsIterator* newIterator(){
        BlockOffsetsIterator* iterator = new BlockOffsetsIterator(block_offset_index_, indexblock_);
        return iterator;
    }

private:
    char* raw_ptr_;
    std::unique_ptr<File> file_;
    pb::IndexBlock indexblock_;
    std::string min_key_;
    std::string max_key_;
    std::unique_ptr<std::string> bloom_filter_;
    uint32_t block_offset_index_;
};

#endif