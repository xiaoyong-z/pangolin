#ifndef LEVELHANDLER_H
#define LEVELHANDLER_H
#include "levelsManager.h"

class LevelHandler {
public:
    LevelHandler(int level_num): level_num_(level_num) {};

    RC level0Get(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt);
    RC levelNGet(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt);
    void flush(const std::shared_ptr<Table>& table);
    const int getTableNum() const;
    const uint64_t getLevelSize() const;

    std::vector<std::shared_ptr<Table>>& getTables();

    void Lock();
    void Unlock();

private:
    std::mutex mutex_;
    int level_num_;
    std::vector<std::shared_ptr<Table>> tables_;
    std::atomic<uint64_t> level_size_;
};

#endif