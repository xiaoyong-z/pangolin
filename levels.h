#ifndef LEVELS_H
#define LEVELS_H
#include "file.h"
#include "memtable.h"
#include "builder.h"
#include "table.h"
class LevelManager {
public:
    LevelManager(): cur_file_id_(0) {}
    static LevelManager* newLevelManager(const std::shared_ptr<Options>& opt){
        LevelManager* level_manger = new LevelManager();
        level_manger->opt = opt;
        // lm.opt = opt
        // // 读取manifest文件构建管理器
        // if err := lm.loadManifest(); err != nil {
        //     panic(err)
        // }
        // lm.build()
        // return lm
        return level_manger;
    }

    RC flush(const MemTable& immutable) {
        uint64_t file_id = cur_file_id_.fetch_add(1);
        Table* table_raw = Table::NewTable(opt->work_dir_, file_id, opt->ssTable_max_sz_);
        if (table_raw == nullptr) {
            return RC::LEVELS_FILE_NOT_OPEN;
        }
        std::unique_ptr<Table> table(table_raw);
        SkipListIterator<std::string, std::string>* ptr = immutable.skipList_->NewIterator();
        std::unique_ptr<Builder> builder = std::make_unique<Builder>(opt);
        std::unique_ptr<STRSkipListIterator> iterator(ptr);
        for (; iterator->end() == false; iterator->next()) {
            builder->insert(iterator->entry());
        }

        
        return RC::SUCCESS;
    }


private:
    std::atomic<uint64_t> cur_file_id_;
    std::shared_ptr<Options> opt;
};

class levelHandler {
    // int level_num;
    // table
};
#endif