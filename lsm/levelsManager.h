#ifndef LEVELS_H
#define LEVELS_H
#include "memtable.h"
#include "builder.h"
#include "table.h"
#include "cache.h"
#include "manifest.h"
#include "levelHandler.h"
#include "iterator.h"
#include "file.h"
class LevelHandler;
class ManifestFile;
class LevelsManager: public std::enable_shared_from_this<LevelsManager> {
    friend class Table;
    friend class LSM;
public:

    LevelsManager(const std::shared_ptr<Options>& options, std::shared_ptr<ManifestFile> manifest_file);
    RC get(const Slice& key, Entry& entry);
    RC flush(std::shared_ptr<MemTable>& memtable); 
    std::shared_ptr<Table> newTable();
    void setLevel0();
    uint64_t getLevelSize(int level_num);
    std::vector<std::shared_ptr<Table>>& getTables(int level_num);
    std::shared_ptr<LevelHandler>& getLevelHandler(int level_num);
    std::shared_ptr<ManifestFile>& getManifestFile();
    void scan();

private:
    std::atomic<uint32_t> cur_file_id_;
    std::shared_ptr<Options> opt_;
    std::vector<std::shared_ptr<LevelHandler>> levels_;
    std::shared_ptr<ManifestFile> manifest_file_;
    Cache cache;
};


#endif