#ifndef BLOCK_H
#define BLOCK_H
#include <vector>
#include "coding.h"
#include "crc32c.h"
class Block {
public:
    Block(): base_key_(""), current_offset(0) {}

    int diffKey(const std::string& key) {
        int min_length = std::min(key.size(), base_key_.size());
        for(int i = 0; i < min_length; i++) {
            if (base_key_[i] != key[i]) {
                return i;
            }
        }
        return min_length;
    }

    RC insert(const strEntry& entry) {
        int diff_key_index;
        if (offset_.size() == 0) {
            base_key_ = entry.key_;
            diff_key_index = base_key_.size();
        } else {
            diff_key_index = diffKey(entry.key_);
        }
        uint32_t header = diff_key_index | (base_key_.size() - diff_key_index) << 16;
        EncodeFix32(&content, header);
        content.append(entry.key_.data() + diff_key_index, base_key_.size() - diff_key_index);
        EncodeFix64(&content, entry.expires_at_);
        content.append(entry.value_, entry.value_.size());

        offset_.push_back(current_offset);
        current_offset += content.size();
        return RC::SUCCESS;
    }

    inline bool checkFinish(const strEntry& entry, int max_size) {
        int offset_crc_size = (current_offset + 1) * 4 + 4 + 8 + 4;
        int estimate_size = content.size() + offset_crc_size + entry.key_.size() + entry.value_.size() + 8;
        return estimate_size > max_size;
    }

    void Finish() {
        for (size_t i = 0; i < offset_.size(); i++) {
            EncodeFix32(&content, offset_[i]);
        }
        EncodeFix32(&content, offset_.size());
        uint32_t crc = crc32c::Value(content.data(), content.size());
        EncodeFix32(&content, crc);
        EncodeFix32(&content, 4);
    }

private:
    std::string content;
    std::vector<uint32_t> offset_;

    std::string base_key_;
    uint32_t current_offset;

};
#endif