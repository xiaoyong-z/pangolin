
#include "table.h"
Table::Table(SSTable* sstable_file, uint32_t file_id): sstable_(sstable_file), fd_(file_id), crc_(0), refs_(0) {}

Table* Table::NewTable(const std::string& dir_name, uint32_t file_id, uint64_t sstable_max_sz) {
    std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
    file_opt->file_name_ = Util::filePathJoin(dir_name, file_id, SSTableConfig::filePostfix);

    file_opt->dir_ = dir_name;
    file_opt->flag_ = O_CREAT | O_RDWR;
    file_opt->max_sz_ = sstable_max_sz;
    SSTable* sstable_file = SSTable::newSSTableFile(file_opt);
    if (sstable_file == nullptr) {
        return nullptr;
    }
    Table* table = new Table(sstable_file, file_id);
    return table;
}

RC Table::flush(const std::shared_ptr<Builder>& builder, bool sync) {
    return builder->flush(sstable_.get(), crc_, sync);
}

RC Table::sync() {
    return sstable_->sync();
}

RC Table::get(std::shared_ptr<Table>& table, const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {
    std::shared_ptr<SSTable>& sstable = table->getSSTable();
    std::string key2 = key.ToString();
    if (Util::compareKey(key2, sstable->getMinKey()) < 0 || Util::compareKey(key2, sstable->getMaxKey()) > 0) {
        return RC::TABLE_EXCEED_MINMAX;
    }

    if (BloomFilter::contains(key.data(), *sstable->getFilter(), key.size() - META_SIZE) == false) {
        return RC::TABLE_BLOOM_FILTER_NOT_CONTAIN;
    }

    std::unique_ptr<TableIterator> iterator = std::make_unique<TableIterator>(table);
    if (iterator->Seek(key2) == false) {
        return RC::TABLE_KEY_NOT_FOUND_IN_BLOCK;   
    }
    iterator->getEntry(entry);
    return RC::SUCCESS;
}

RC Table::open() {
    sstable_->init(); 
    return RC::SUCCESS;
}

uint64_t Table::getCacheBlockId(uint32_t block_index) {    
    return static_cast<uint64_t>(block_index) | static_cast<uint64_t>(fd_) << 32;
}

uint32_t Table::getFD() {
    return fd_;
}

uint64_t Table::getSize() {
    return sstable_->getSize();
}

uint32_t Table::getBlockCount() {
    return sstable_->getBlockCount();
}

uint32_t Table::getCRC() {
    return sstable_->getCRC();;
}

std::string& Table::getMinKey() {
    return sstable_->getMinKey();
}

std::string& Table::getMaxKey() {
    return sstable_->getMaxKey();
}

int Table::getMaxVersion() {
    return sstable_->getMaxVersion();
}

void Table::increaseRef() {
    refs_.fetch_add(1);
}

void Table::decreaseRef() {
    refs_.fetch_sub(1);
}

std::shared_ptr<SSTable>& Table::getSSTable() {
    return sstable_;
}

const pb::IndexBlock& Table::getIndexBlock() {
    return sstable_->getIndexBlock();
}