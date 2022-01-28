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

    RC updateList(const std::shared_ptr<Options>& options) {
        if (wal_file_.get() == nullptr || skipList_.get() == nullptr) {
            return RC::MEMTABLE_UNINTIALIZE_FAIL;
        }
        
        wal_file_->iterator(true, 0, skipList_.get(), replayFunction);
        return RC::SUCCESS;
    }

    static void replayFunction(SkipList* skip_list, Entry* entry) {
        RC rc = skip_list->insert(entry);
        assert(rc == RC::SUCCESS);
    }

    void close() {
        wal_file_->close();
    }

    int getEntryCount() {
        return skipList_->getEntryCount();
    }

    std::unique_ptr<WALFile> wal_file_;
    std::unique_ptr<SkipList> skipList_;
};
#endif