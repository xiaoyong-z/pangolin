#ifndef SSTABLE_FILE_H
#define SSTABLE_FILE_H

#include "file.h"
#include "util.h"
#include "kv.pb.h"
#include "iterator.h"
#include "mmapfile.h"
class SSTable {
    friend class Block;
private:
    SSTable(MmapFile* mmap_file): file_(mmap_file), max_version_(0){}
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

    RC sync() {
        return file_->sync();
    }
    
    RC init(uint32_t& crc, uint64_t& size) {
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
        crc = check_sum;
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
        
        size_ = indexblock_.offsets_size();
        if (size_ == 0) {
            return RC::SUCCESS;
        }
        assert(size_ > 0);

        min_key_ = indexblock_.offsets(0).base_key();
        //min_key_ = Slice(indexblock_.offsets(0).base_key());

        std::string max_block_base_key = indexblock_.offsets(size_ - 1).base_key();
        uint64_t max_block_offset = indexblock_.offsets(size_ - 1).offset();
        uint64_t max_block_len = indexblock_.offsets(size_ - 1).len();

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
        size = indexblock_.offsets().size();
        return RC::SUCCESS;
    }

    inline std::string& getMinKey() {
        return min_key_;
    }

    inline std::string& getMaxKey() {
        return max_key_;
    }

    inline uint32_t getMaxVersion() {
        return max_version_;
    }

    inline const pb::IndexBlock& getIndexBlock() {
        return indexblock_;
    }

    inline uint32_t getSize() {
        return size_;
    }

    inline std::string* getFilter() {
        return bloom_filter_.get();
    }

private:
    char* raw_ptr_;
    std::unique_ptr<File> file_;
    pb::IndexBlock indexblock_;
    std::string min_key_;
    std::string max_key_;
    std::unique_ptr<std::string> bloom_filter_;
    uint32_t max_version_;
    uint32_t size_;
};

#endif