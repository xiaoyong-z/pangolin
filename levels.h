#ifndef LEVELS_H
#define LEVELS_H
#include "file.h"
#include "memtable.h"
#include "builder.h"
class LevelManager {

public:
    static LevelManager* newLevelManager(const FileOptions& opt){
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
        
        SkipListIterator<std::string, std::string>* ptr = immutable.skipList_->NewIterator();
        std::unique_ptr<Builder> builder = std::make_unique<Builder>();
        std::unique_ptr<STRSkipListIterator> iterator(ptr);
        for (; iterator->end() == false; iterator->next()) {
            builder->insert(iterator->entry());
        }
        return RC::SUCCESS;
    }


private:
    uint64_t max_fid;
    FileOptions opt;
};

class levelHandler {
    // int level_num;
    // table
};
#endif