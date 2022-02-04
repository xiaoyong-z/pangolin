#include "levelHandler.h"


RC LevelHandler::level0Get(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {
    for (size_t i = tables_.size() - 1; i >= 0; i--) {
        if (Table::get(tables_[i], key, entry, opt) == RC::SUCCESS) {
            return RC::SUCCESS;
        }
    }
    return RC::LEVELS_KEY_NOT_FOUND_IN_CUR_LEVEL;
}

RC LevelHandler::levelNGet(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {
    return RC::SUCCESS;
}

void LevelHandler::appendTable(const std::shared_ptr<Table>& table) {
    WLock();
    tables_.push_back(table);
    level_size_.fetch_add(table->getSize());
    UnWLock();
}

const int LevelHandler::getTableNum() {
    RLock();
    int result = tables_.size();
    UnRLock();
    return result;
}

const uint64_t LevelHandler::getLevelSize() {
    RLock();
    uint64_t level_size = level_size_.load();
    UnRLock();
    return level_size; 
}

std::vector<std::shared_ptr<Table>>& LevelHandler::getTables() {
    return tables_;
}

void LevelHandler::RLock() {
    rwLock_.lock_shared();
}

void LevelHandler::UnRLock() {
    rwLock_.unlock_shared();
}

void LevelHandler::WLock() {
    rwLock_.lock();
}

void LevelHandler::UnWLock() {
    rwLock_.unlock();
}

void LevelHandler::replaceTables(std::vector<std::shared_ptr<Table>>& old_tables, std::vector<std::shared_ptr<Table>>& new_tables) {
    WLock();
    std::vector<std::shared_ptr<Table>> temp_tables;
    for (size_t i = 0; i < tables_.size(); i++) {
        bool contain = false;
        for (size_t j = 0; j < old_tables.size(); j++) {
            if (old_tables[j] == tables_[i]) {
                contain = true;
                old_tables.erase(old_tables.begin() + j);
                break;
            }
        }
        if (contain == false) {
            temp_tables.push_back(tables_[i]);
        }
    }
    for (size_t i = 0; i < new_tables.size(); i++) {
        temp_tables.push_back(new_tables[i]);
    }
    tables_.clear();
    tables_.assign(temp_tables.begin(), temp_tables.end());
    sortTables();
    UnWLock();
}

void LevelHandler::deleteTables(std::vector<std::shared_ptr<Table>>& old_tables) {
    WLock();
    std::vector<std::shared_ptr<Table>> temp_tables;
    for (size_t i = 0; i < tables_.size(); i++) {
        bool contain = false;
        for (size_t j = 0; j < old_tables.size(); j++) {
            if (old_tables[j] == tables_[i]) {
                contain = true;
                old_tables.erase(old_tables.begin() + j);
                break;
            }
        }
        if (contain == false) {
            temp_tables.push_back(tables_[i]);
        }
    }
    tables_.clear();
    tables_.assign(temp_tables.begin(), temp_tables.end());
    sortTables();
    UnWLock();
}

void LevelHandler::scan() {
    RLock();
    for (size_t i = 0; i < tables_.size(); i++) {
        std::cout << "scan table i: " << i << ". " << std::endl; 
        std::unique_ptr<TableIterator> iterator = std::make_unique<TableIterator>(tables_[i]);
        while (iterator->Valid()) {
            if (BloomFilter::contains(iterator->getKey().data(), *tables_[i]->getSSTable()->getFilter(), iterator->getKey().size() - META_SIZE) == false) {
                std::cout << "warning: " << iterator->getKey().data() << " not in the bloom filter" << std::endl;
            }
            std::cout << "key: " << iterator->getKey() << ", value: " << iterator->getValue() << std::endl;
            iterator->Next();
        }
        std::cout << "scan table i " << i << " finish . " << std::endl; 
    }
    UnRLock();
}

// before calling this function, make sure acuqire the write lock first
void LevelHandler::sortTables() {
    if (level_num_ == 0) {
        std::sort(tables_.begin(), tables_.end(), [](const std::shared_ptr<Table>& a, const std::shared_ptr<Table>& b) {
            return a->getFD() < b->getFD();
        });
    } else {
        std::sort(tables_.begin(), tables_.end(), [](const std::shared_ptr<Table>& a, const std::shared_ptr<Table>& b) {
            return Util::compareKey(a->getMinKey(), b->getMinKey()) > 0;
        });
    }
}