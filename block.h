#ifndef BLOCK_H
#define BLOCK_H
#include <vector>
#include "coding.h"
#include "crc32c.h"
#include "kv.pb.h"
#include "sstable.h"
#include "util.h"
class BlockIterator {

public:
    BlockIterator(const std::string& base_key, const std::vector<uint32_t>& offset, const std::string& content, const uint32_t keys_len): 
        base_key_(base_key), offset_(offset), content_(content), current_offset_index_(0), keys_len_(keys_len) {}

    bool hasNext() {
        return current_offset_index_ != offset_.size();
    }

    void next() {
        current_offset_index_++;
    }
    
    const std::string get() {
        uint32_t cur_offset = offset_[current_offset_index_];
        uint32_t header = DecodeFix32(content_.data() + cur_offset);
        uint16_t overlap = header;
        uint16_t diff = header >> 16;
        std::string diff_key(content_.data() + cur_offset + 4, diff);
        std::string value(base_key_.substr(0, overlap) + diff_key);
        return value;
    }

    const std::string find(const std::string& key) {
        uint32_t min = 0, max = offset_.size();
        while (min <= max) {
            uint32_t mid = min + (max - min) / 2;
            std::string key2 = getKey(mid);
            if (key == key2) {
                return getValue(mid);
            } else if (key > key2) {
                min = mid + 1;
            } else {
                max = mid - 1;
            }
        }
        return "";
    }
private:
    const std::string getKey(uint32_t index) {
        uint32_t cur_offset = offset_[index];
        uint32_t header = DecodeFix32(content_.data() + cur_offset);
        uint16_t overlap = header;
        uint16_t diff = header >> 16;
        std::string diff_key(content_.data() + cur_offset + 4, diff);
        std::string value(base_key_.substr(0, overlap) + diff_key);
        return value;
    }

    const std::string getValue(uint32_t index) {
        uint32_t cur_offset = offset_[index];
        uint32_t header = DecodeFix32(content_.data() + cur_offset);
        uint16_t diff = header >> 16;
        if (index < offset_.size() - 1) {
            uint32_t next_offset = offset_[index + 1];
            std::string value(content_.data() + cur_offset + 4 + diff + 8, next_offset - (cur_offset + 4 + diff + 8));
            return value;
        } else {
            std::string value(content_.data() + cur_offset + 4 + diff + 8, keys_len_ - (cur_offset + 4 + diff + 4));
            return value;
        }
    }

    const std::string& base_key_;
    const std::vector<uint32_t>& offset_;
    const std::string& content_;
    uint32_t current_offset_index_;
    const uint32_t keys_len_;
};

class Block {
public:
    friend class Builder;
    Block(): base_key_(""), current_offset_(0) {}
    Block(const pb::BlockOffset& blockOffset, SSTable* sstable) {
        uint32_t block_len = blockOffset.len();
        current_offset_ = block_len;
        uint64_t offset = blockOffset.offset();
        std::string block_content_(sstable->raw_ptr_ + SSTABLE_SIZE_LEN + offset, block_len);
        uint32_t block_check_sum_len = DecodeFix32(block_content_.data() + block_content_.size() - 4);
        uint32_t block_check_sum = DecodeFix32(block_content_.data() + block_content_.size() - 4 - block_check_sum_len);
        uint32_t block_crc_value = crc32c::Value(block_content_.data(), block_content_.size() - 4 - block_check_sum_len);
        assert (block_crc_value == block_check_sum);
        uint32_t offset_len = DecodeFix32(block_content_.data() + block_content_.size() - 4 - block_check_sum_len - 4);
        for (uint32_t i = 0; i < offset_len; i++) {
            offset_.push_back(DecodeFix32(block_content_.data() + block_content_.size() - 4 - block_check_sum_len - 4 - (offset_len - i) * 4));
        }
        base_key_ = blockOffset.base_key();
        content_ = std::move(block_content_);
    }

    int diffKey(const Slice& key) {
        int min_length = std::min(key.size(), base_key_.size());
        for(int i = 0; i < min_length; i++) {
            if (base_key_.data()[i] != key.data()[i]) {
                return i;
            }
        }
        return min_length;
    }

    RC insert(const Entry& entry) {
        int diff_key_index;
        if (offset_.size() == 0) {
            base_key_ = std::move(entry.key_.ToString());
            diff_key_index = base_key_.size();
        } else {
            diff_key_index = diffKey(entry.key_);
        }
        uint32_t header = diff_key_index | (base_key_.size() - diff_key_index) << 16;
        EncodeFix32(&content_, header);
        content_.append(entry.key_.data() + diff_key_index, base_key_.size() - diff_key_index);
        EncodeFix64(&content_, entry.expires_at_);
        content_.append(entry.value_.data(), entry.value_.size());

        offset_.push_back(current_offset_);
        current_offset_ = content_.size();
        return RC::SUCCESS;
    }

    inline bool checkFinish(const Entry& entry, int max_size) {
        int offset_crc_size = (offset_.size() + 1) * 4 + 4 + 8 + 4;
        int estimate_size = content_.size() + offset_crc_size + entry.key_.size() + entry.value_.size() + 8;
        return estimate_size > max_size;
    }

    inline uint32_t GetKeyCount() {
        return offset_.size();
    }

    void Finish() {
        for (size_t i = 0; i < offset_.size(); i++) {
            EncodeFix32(&content_, offset_[i]);
        }
        EncodeFix32(&content_, offset_.size());
        uint32_t crc = crc32c::Value(content_.data(), content_.size());
        EncodeFix32(&content_, crc);
        EncodeFix32(&content_, sizeof(crc));
        current_offset_ = content_.size();
    }

    BlockIterator* NewIterator() {
        return new BlockIterator(base_key_, offset_, content_, current_offset_ - 4 * offset_.size() - 4 - 4 - CRC_SIZE_LEN);
    }

private:
    std::string content_;
    std::vector<uint32_t> offset_;

    std::string base_key_;
    uint32_t current_offset_;
};
#endif