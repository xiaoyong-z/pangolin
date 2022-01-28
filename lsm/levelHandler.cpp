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