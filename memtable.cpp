#include "skipLists.h"
#include <memory>
#include "wal.h"
#include "util.h"
class MemTable {
public: 
    MemTable(std::unique_ptr<WALFile>&& wal_file, std::unique_ptr<STRSkipList>&& skiplist): 
        wal_file_(std::move(wal_file)), skipList_(std::move(skiplist)){};
    
    RC set(std::shared_ptr<Entry<std::string, std::string>> entry) {
        RC result = wal_file_->Write(entry);
        if (result != RC::SUCCESS) {
            return result;
        }
        // RC result = skipList_->
    }
    
    std::unique_ptr<WALFile> wal_file_;
    std::unique_ptr<STRSkipList> skipList_;
};