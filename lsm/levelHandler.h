#ifndef LEVELHANDLER_H
#define LEVELHANDLER_H
#include "levelsManager.h"

class LevelHandler {
public:
    LevelHandler() = default;

    RC level0Get(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt);
    RC levelNGet(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt);
    
private:
    friend class LevelsManager;
    int level_num_;
    std::vector<std::shared_ptr<Table>> tables_;
    std::atomic<uint64_t> level_size_;
};

#endif