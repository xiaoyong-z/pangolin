#include "wal.h"

RC WALFile::iterator(bool read_only, uint64_t start_offset, SkipList* skip_list, void (*func)(SkipList* skip_list, Entry* entry)) {
    if (file_ == nullptr) {
        return RC::WAL_UNINTIALIZE;
    }
    char* data;
    RC result = file_->get_mmap_ptr(data);
    std::shared_ptr<MmapFile::MmapReader> reader = std::make_shared<MmapFile::MmapReader>(file_.get());
    
    Entry entry;
    while (decodeEntry(reader, &entry) == RC::SUCCESS) {
        func(skip_list, &entry);
    }
    
    
    // char* buf;
    // uint64_t n = reader->read(12, buf);
    // if (n)
    // std::cout << n << std::endl;

    // reader->init();
    // file_.get(read_only);
}