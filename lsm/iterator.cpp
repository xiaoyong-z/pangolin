#include "iterator.h"

SkipListIterator::SkipListIterator(const std::shared_ptr<SkipList>& skiplist):
    it_(skiplist->firstNode()), skip_list_(skiplist) {}

std::shared_ptr<SkipListIterator> SkipListIterator::NewIterator(const std::shared_ptr<SkipList>& skiplist) {
    std::shared_ptr<SkipListIterator> iterator = std::make_shared<SkipListIterator>(SkipListIterator(skiplist));
    return iterator;
}

SkipListIterator::~SkipListIterator() {}

void SkipListIterator::Next() {
    it_ = it_->next(0);
}

bool SkipListIterator::Valid() {
    return it_ != nullptr;
}

void SkipListIterator::Close() {

}

void SkipListIterator::Rewind() {
    it_ = skip_list_->firstNode();
}

bool SkipListIterator::Seek(const std::string& key) {
    Rewind();
    while (Valid()) {
        int cmp = memcmp(key.data(), it_->key_, std::min(static_cast<uint32_t>(key.size()), it_->key_len_));
        if (cmp > 0) {
            break;
        } else if(cmp == 0) {
            return true;
        } else {
            Next();
        }
    }
    return false;
}

std::string SkipListIterator::getKey() {
    return skip_list_->getKey(it_->key_, it_->key_len_).ToString();
}

void SkipListIterator::getEntry(Entry& entry) {
    const Slice key_find = skip_list_->getKey(it_->key_, it_->key_len_);
    const ValueStruct value_find = skip_list_->getValue(it_->value_, it_->value_len_);
    entry.reset(key_find, value_find);
}

BlockIterator::BlockIterator(const std::shared_ptr<Block>& block): block_(block), 
    base_key_(block->getBaseKey()), offset_(block->getOffsets()), content_(block->getContent()), pos_(0), 
        end_(offset_.size()), keys_len_(block->getSize() - 4 * offset_.size() - 4 - 4 - CRC_SIZE_LEN) {}

BlockIterator::~BlockIterator() {}

std::shared_ptr<BlockIterator> NewIterator(const std::shared_ptr<Block>& block) {
    std::shared_ptr<BlockIterator> iterator = std::make_shared<BlockIterator>(block);
    return iterator;
}

void BlockIterator::Close() {
    
}

void BlockIterator::Rewind() {
    pos_ = 0;
}

bool BlockIterator::Seek(const std::string& key) {
    uint32_t left = 0, right = offset_.size();
    while (left <= right) {
        uint32_t mid = left + (right - left) / 2;
        std::string key2 = getKey(mid);
        int cmp = Util::compareKey(key, key2);
        if (cmp == 0) {
            pos_ = mid;
            return true;
        } else if (cmp > 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    pos_ = left;
    return false;
}

bool BlockIterator::Valid() {
    return pos_ != end_;
}

void BlockIterator::Next() {
    assert(Valid());
    pos_++;
}

std::string BlockIterator::getKey() {
    uint32_t cur_offset = offset_[pos_];
    uint32_t header = decodeFix32(content_.data() + cur_offset);
    uint16_t overlap = header;
    uint16_t diff = header >> 16;
    std::string diff_key(content_.data() + cur_offset + 4, diff);
    std::string value(base_key_.substr(0, overlap) + diff_key);
    return value;
}

void BlockIterator::getEntry(Entry& entry) {
    const std::string value = getValue(pos_);
    char* buf = new char[value.size() + 1];
    memcpy(buf, value.data(), value.size());
    buf[value.size()] = '\0';
    entry.value_.reset(buf, value.size());
}

const std::string BlockIterator::getKey(uint32_t index) {
    uint32_t cur_offset = offset_[index];
    uint32_t header = decodeFix32(content_.data() + cur_offset);
    uint16_t overlap = header;
    uint16_t diff = header >> 16;
    std::string diff_key(content_.data() + cur_offset + 4, diff);
    std::string key(base_key_.substr(0, overlap) + diff_key);
    return key;
}

const std::string BlockIterator::getValue(uint32_t index) {
    uint32_t cur_offset = offset_[index];
    uint32_t header = decodeFix32(content_.data() + cur_offset);
    uint16_t diff = header >> 16;
    if (index < offset_.size() - 1) {
        uint32_t next_offset = offset_[index + 1];
        std::string value(content_.data() + cur_offset + 4 + diff + 8, next_offset - (cur_offset + 4 + diff + 8));
        return value;
    } else {
        std::string value(content_.data() + cur_offset + 4 + diff + 8, keys_len_ - (cur_offset + 4 + diff + 8));
        return value;
    }
}

TableIterator::TableIterator(const std::shared_ptr<Table>& table): 
    table_(table), pos_(0), end_(table->getSize()), index_block_(table_->getIndexBlock()) {
    updateBlock();
}

std::shared_ptr<TableIterator> NewIterator(const std::shared_ptr<Table>& table) {
    std::shared_ptr<TableIterator> iterator = std::make_shared<TableIterator>(table);
    return iterator;
}

TableIterator::~TableIterator() {}

void TableIterator::Close() {}

void TableIterator::Rewind() {
    assert(end_ > 0);
    pos_ = 0;
    updateBlock();
}

bool TableIterator::Seek(const std::string& key) {
    uint32_t index = getBlockIndex(key);
    updateBlock(index);
    return block_iterator_->Seek(key);
}

bool TableIterator::Valid() {
    if (block_iterator_->Valid()) {
        return true;
    }
    return pos_ < end_;
}

void TableIterator::Next() {
    if (block_iterator_->Valid()) {
        block_iterator_->Next();
    }
    if (pos_ < end_) {
        pos_++;
        updateBlock();
    }
} 

std::string TableIterator::getKey() {
    assert (Valid());
    return block_iterator_->getKey();
}

void TableIterator::getEntry(Entry& entry) {
    assert (Valid());
    return block_iterator_->getEntry(entry);
}

const pb::BlockOffset& TableIterator::getBlockOffset(uint32_t index) {
        return index_block_.offsets(index);
    }

const uint32_t TableIterator::getBlockIndex(const std::string& key) {
    uint32_t left = 0, right = end_ - 1;
    while(left < right) {
        uint32_t mid = left + (right - left) / 2 + 1;
        assert(mid >= 0 && mid <= end_ - 1);
        const pb::BlockOffset& blockOffset = index_block_.offsets(mid);
        int cmp = Util::compareKey(blockOffset.base_key(), key);
        if (cmp == 0) {
            return mid;
        } else if (cmp < 0) {
            left = mid;
        } else {
            right = mid - 1;
        }
    }
    return left;
}

inline void TableIterator::updateBlock() {
    updateBlock(pos_);
}

void TableIterator::updateBlock(uint32_t pos) {
    const pb::BlockOffset& blockOffset = getBlockOffset(pos); 
    std::shared_ptr<Block> block = std::make_shared<Block>(blockOffset, table_->getSSTable().get());
    block_iterator_ = std::make_shared<BlockIterator>(block);
}