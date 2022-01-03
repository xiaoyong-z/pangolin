#ifndef LEVELS_H
#define LEVELS_H
#include "file.h"
#include "memtable.h"
#include "builder.h"
#include "table.h"
#include "cache.h"
#include "manifest.h"
class LevelManager;
class LevelHandler {
public:
    RC level0Get(const std::string& key, Entry& entry, const std::shared_ptr<Options>& opt) {
        for (size_t i = 0; i < tables_.size(); i++) {
            if (tables_[i]->get(key, entry, opt) == RC::SUCCESS) {
                entry.key_ = key;
                return RC::SUCCESS;
            }
        }
        return RC::LEVELS_KEY_NOT_FOUND_IN_CUR_LEVEL;
    }

    RC levelNGet(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {

        return RC::SUCCESS;
    }
private:
    friend class LevelManager;
    int level_num_;
    std::vector<std::shared_ptr<Table>> tables_;
};

class LevelManager: public std::enable_shared_from_this<LevelManager> {
    friend class Table;
    friend class LSM;
public:
    LevelManager(const std::shared_ptr<Options>& options): cur_file_id_(0), opt_(options) {
        levels_.emplace_back();
    }

    RC get(const std::string& key, Entry& entry) {
        RC result = levels_[0].level0Get(key, entry, opt_);
        return RC::SUCCESS;
    }

    RC flush(const std::shared_ptr<MemTable>& memtable) {
        uint32_t file_id = cur_file_id_.fetch_add(1);
        Table* table_raw = Table::NewTable(opt_->work_dir_, file_id, opt_->ssTable_max_sz_);
        if (table_raw == nullptr) {
            return RC::LEVELS_FILE_NOT_OPEN;
        }
        std::shared_ptr<Table> table(table_raw);
        std::shared_ptr<Builder> builder = std::make_shared<Builder>(opt_);
        std::unique_ptr<SkipListIterator> iterator(memtable->skipList_->newIterator());
        for (; iterator->hasNext() ; iterator->next()) {
            builder->insert(iterator->get());
        }
        table_raw->flush(builder);
        table_raw->open();
        levels_[0].tables_.push_back(table);

        return RC::SUCCESS;
    }

private:
    std::atomic<uint32_t> cur_file_id_;
    std::shared_ptr<Options> opt_;
    std::vector<LevelHandler> levels_;
    std::unique_ptr<Manifest> manifest_;
    Cache cache;
};


#endif