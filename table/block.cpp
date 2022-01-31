#include "block.h"
Block::Block(): base_key_(""), size_(0) {

}

Block::Block(const pb::BlockOffset& blockOffset, SSTable* sstable) {
    uint32_t block_len = blockOffset.len();
    size_ = block_len;
    uint64_t offset = blockOffset.offset();
    std::string block_content_(sstable->raw_ptr_ + SSTABLE_SIZE_LEN + offset, block_len);
    uint32_t block_check_sum_len = decodeFix32(block_content_.data() + block_content_.size() - 4);
    uint32_t block_check_sum = decodeFix32(block_content_.data() + block_content_.size() - 4 - block_check_sum_len);
    uint32_t block_crc_value = crc32c::Value(block_content_.data(), block_content_.size() - 4 - block_check_sum_len);
    assert (block_crc_value == block_check_sum);
    uint32_t offset_len = decodeFix32(block_content_.data() + block_content_.size() - 4 - block_check_sum_len - 4);
    for (uint32_t i = 0; i < offset_len; i++) {
        offset_.push_back(decodeFix32(block_content_.data() + block_content_.size() - 4 - block_check_sum_len - 4 - (offset_len - i) * 4));
    }
    base_key_ = blockOffset.base_key();
    content_ = std::move(block_content_);
}

int Block::diffKey(const Slice& key) {
    int min_length = std::min(key.size(), base_key_.size());
    for(int i = 0; i < min_length; i++) {
        if (base_key_.data()[i] != key.data()[i]) {
            return i;
        }
    }
    return min_length;
}

RC Block::insert(const Entry& entry) {
    int diff_key_index;
    if (offset_.size() == 0) {
        base_key_ = std::move(entry.key_.ToString());
        diff_key_index = base_key_.size();
    } else {
        diff_key_index = diffKey(entry.key_);
    }
    uint32_t header = diff_key_index | (base_key_.size() - diff_key_index) << 16;
    encodeFix32(&content_, header);
    content_.append(entry.key_.data() + diff_key_index, base_key_.size() - diff_key_index);
    encodeFix64(&content_, entry.expires_at_);
    content_.append(entry.value_.data(), entry.value_.size());

    offset_.push_back(size_);
    size_ = content_.size();
    return RC::SUCCESS;
}

bool Block::checkFinish(const Entry& entry, int max_size) {
    int offset_crc_size = (offset_.size() + 1) * 4 + 4 + 8 + 4;
    int estimate_size = content_.size() + offset_crc_size + entry.key_.size() + entry.value_.size() + 8;
    return estimate_size > max_size;
}

uint32_t Block::getKeyCount() {
    return offset_.size();
}

void Block::finish() {
    for (size_t i = 0; i < offset_.size(); i++) {
        encodeFix32(&content_, offset_[i]);
    }
    encodeFix32(&content_, offset_.size());
    uint32_t crc = crc32c::Value(content_.data(), content_.size());
    encodeFix32(&content_, crc);
    encodeFix32(&content_, sizeof(crc));
    size_ = content_.size();
}

std::string& Block::getBaseKey() {
    return base_key_;
}

std::string& Block::getContent() {
    return content_;
}

std::vector<uint32_t>& Block::getOffsets() {
    return offset_;
}

uint32_t Block::getSize() {
    return size_;
}

