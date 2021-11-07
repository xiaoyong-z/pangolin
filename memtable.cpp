#include "skipLists.h"
#include <memory>
#include "wal.h"
class MemTable {
public: 
    MemTable(std::unique_ptr<WALFile>&& wal_file, std::unique_ptr<STRSkipList>&& skiplist): 
        wal_file_(std::move(wal_file)), skipList_(std::move(skiplist)){};

    
    std::unique_ptr<WALFile> wal_file_;
    std::unique_ptr<STRSkipList> skipList_;
};