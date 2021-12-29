#ifndef MEMTABLE_H
#define MEMTABLE_H
#include <memory>
#include <filesystem>
#include "wal.h"
#include "util.h"
#include "skipLists.h"
class MemTable {
public: 
    MemTable(std::unique_ptr<WALFile>&& wal_file, std::unique_ptr<SkipList>&& skiplist): 
        wal_file_(std::move(wal_file)), skipList_(std::move(skiplist)){};

    

    static RC OpenWALFiles(std::shared_ptr<FileOptions> file_opt) {
        
    }
    
    RC set(Entry* entry) {
        RC result = wal_file_->Write(entry);
        if (result != RC::SUCCESS) {
            return result;
        }
        result = skipList_->Insert(entry);
        return RC::SUCCESS;
    }

    RC get(const Slice& key, Entry& result) {
        return skipList_->Contains(key, result);
    }

    std::unique_ptr<WALFile> wal_file_;
    std::unique_ptr<SkipList> skipList_;
};
#endif