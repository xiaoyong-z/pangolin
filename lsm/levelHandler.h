#ifndef LEVELHANDLER_H
#define LEVELHANDLER_H
#include "levelsManager.h"

class LevelHandler {
public:
    LevelHandler(int level_num): level_num_(level_num) {};

    RC level0Get(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt);
    RC levelNGet(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt);
    void appendTable(const std::shared_ptr<Table>& table);
    const int getTableNum();
    const uint64_t getLevelSize();
    void replaceTables(std::vector<std::shared_ptr<Table>>& old_tables, std::vector<std::shared_ptr<Table>>& new_tables);
    void deleteTables(std::vector<std::shared_ptr<Table>>& old_tables);
    std::vector<std::shared_ptr<Table>>& getTables();
    void sortTables();
    void scan();

    void RLock();
    void UnRLock();
    void WLock();
    void UnWLock();

private:
    RWLock rwLock_;
    int level_num_;
    std::vector<std::shared_ptr<Table>> tables_;
    std::atomic<uint64_t> level_size_;
};

#endif