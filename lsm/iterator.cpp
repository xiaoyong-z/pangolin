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
    assert (Valid());
    return skip_list_->getKey(it_->key_, it_->key_len_).ToString();
}

std::string SkipListIterator::getValue() {
    assert (Valid());
    return skip_list_->getValue(it_->value_, it_->value_len_).value_.ToString();
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
    uint32_t left = 0, right = offset_.size() - 1;
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
    assert (Valid());
    uint32_t cur_offset = offset_[pos_];
    uint32_t header = decodeFix32(content_.data() + cur_offset);
    uint16_t overlap = header;
    uint16_t diff = header >> 16;
    std::string diff_key(content_.data() + cur_offset + 4, diff);
    std::string key(base_key_.substr(0, overlap) + diff_key);
    return key;
}

std::string BlockIterator::getValue() {
    assert (Valid());
    return getValue(pos_);
}

void BlockIterator::getEntry(Entry& entry) {
    const std::string value = getValue(pos_);
    char* buf = new char[value.size() + 1];
    memcpy(buf, value.data(), value.size());
    buf[value.size()] = '\0';
    entry.resetValue(buf, value.size());
    
    const std::string key = getKey(pos_);
    buf = new char[key.size() + 1];
    memcpy(buf, key.data(), key.size());
    buf[key.size()] = '\0';
    entry.resetKey(buf, key.size());
    
}

const std::string BlockIterator::getKey(uint32_t index) {
    assert (index < offset_.size());
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
        std::string value(content_.data() + cur_offset + 4 + diff, next_offset - (cur_offset + 4 + diff));
        return value;
    } else {
        std::string value(content_.data() + cur_offset + 4 + diff, keys_len_ - (cur_offset + 4 + diff));
        return value;
    }
}

TableIterator::TableIterator(const std::shared_ptr<Table>& table): 
    table_(table), pos_(0), end_(table->getBlockCount()), index_block_(table_->getIndexBlock()) {
    updateBlock();
}

std::shared_ptr<TableIterator> TableIterator::NewIterator(const std::shared_ptr<Table>& table) {
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
    block_iterator_->Next();
    if (block_iterator_->Valid()) { 
        return;
    }
    pos_++;
    if (Valid()) {
        updateBlock();
    }  
} 

std::string TableIterator::getKey() {
    assert (Valid());
    return block_iterator_->getKey();
}

std::string TableIterator::getValue() {
    assert (Valid());
    return block_iterator_->getValue();
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

TableMergeIterator::TableMergeIterator(const std::vector<std::shared_ptr<Table>>& tables): tables_(tables) {
    Rewind();
}

std::shared_ptr<TableMergeIterator> TableMergeIterator::NewIterator(const std::vector<std::shared_ptr<Table>>& tables) {
    std::shared_ptr<TableMergeIterator> iterator = std::make_shared<TableMergeIterator>(tables);
    return iterator;
}

TableMergeIterator::~TableMergeIterator() {

}

void TableMergeIterator::Close() {

}

void TableMergeIterator::Rewind() {
    iterators_.clear();
    for (size_t i = 0; i < tables_.size(); i++) {
        iterators_.emplace_back(TableIterator::NewIterator(tables_[i]));
    }
    Update();
}

bool TableMergeIterator::Seek(const std::string& key) {
    // unimplemanted
    assert(false);
}

bool TableMergeIterator::Valid() {
    for (size_t i = 0; i < tables_.size(); i++) {
        if (iterators_[i]->Valid()) {
            return true;
        }
    }
    return false;
}

void TableMergeIterator::Next() {
    assert(Valid());
    iterators_[min_iterator_]->Next();
    Update();
}

std::string TableMergeIterator::getKey() {
    assert(Valid());
    return iterators_[min_iterator_]->getKey();
}

std::string TableMergeIterator::getValue() {
    assert(Valid());
    return iterators_[min_iterator_]->getValue();
}

void TableMergeIterator::getEntry(Entry& entry) {
    assert(Valid());
    return iterators_[min_iterator_]->getEntry(entry);
}

void TableMergeIterator::Update() {
    min_iterator_ = -1;
    std::string min_key = "";
    for (size_t i = 0; i < tables_.size(); i++) {
        if (iterators_[i]->Valid() == false) {
            continue;
        }
        std::string key = iterators_[i]->getKey();
        if (min_key == "") {
            min_key = key;
            min_iterator_ = i;
            continue;
        }
        if (Util::compareKey(key, min_key) < 0) {
            min_key = key;
            min_iterator_ = i;
        }
    }  
}