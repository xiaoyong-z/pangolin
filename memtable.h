#ifndef MEMTABLE_H
#define MEMTABLE_H
#include <memory>
#include <filesystem>
#include "wal.h"
#include "util.h"
#include "skipLists.h"
class MemTable {
    friend class LSM;
public: 
    MemTable(std::unique_ptr<WALFile>&& wal_file, std::unique_ptr<SkipList>&& skiplist): 
        wal_file_(std::move(wal_file)), skipList_(std::move(skiplist)){};

    MemTable(WALFile* wal_file, SkipList* skiplist): 
        wal_file_(wal_file), skipList_(skiplist){};
    
    RC set(Entry* entry) {
        RC result = wal_file_->write(entry);
        if (result != RC::SUCCESS) {
            return result;
        }
        result = skipList_->insert(entry);
        return RC::SUCCESS;
    }

    RC get(const Slice& key, Entry& entry) {
        return skipList_->contains(key, entry);
    }

    RC updateList() {
        if (wal_file_.get() == nullptr || skipList_.get() == nullptr) {
            return RC::MEMTABLE_UNINTIALIZE_FAIL;
        }

        
        if (wal_file_.get() == nullptr) {
            
        }
    }

    std::unique_ptr<WALFile> wal_file_;
    std::unique_ptr<SkipList> skipList_;
};
#endif