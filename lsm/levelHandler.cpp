#include "levelHandler.h"


RC LevelHandler::level0Get(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {
    for (size_t i = 0; i < tables_.size(); i++) {
        if (tables_[i]->get(key, entry, opt) == RC::SUCCESS) {
            return RC::SUCCESS;
        }
    }
    return RC::LEVELS_KEY_NOT_FOUND_IN_CUR_LEVEL;
}

RC LevelHandler::levelNGet(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {
    return RC::SUCCESS;
}

void LevelHandler::flush(const std::shared_ptr<Table>& table) {
    tables_.push_back(table);
    level_size_.fetch_add(table->getSize());
}

const int LevelHandler::getTableNum() const {
    return tables_.size();
}

const uint64_t LevelHandler::getLevelSize() const {
    return level_size_.load(); 
}

std::vector<std::shared_ptr<Table>>& LevelHandler::getTables() {
    return tables_;
}

void LevelHandler::Lock() {
    mutex_.lock();
}

void LevelHandler::Unlock() {
    mutex_.unlock();
}