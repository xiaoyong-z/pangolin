#ifndef SSTABLE_FILE_H
#define SSTABLE_FILE_H

#include "file.h"
#include "util.h"
class SSTable {
private:
    SSTable(MmapFile* mmap_file): file_(mmap_file){}
public:
    static SSTable* NewSSTableFile(const std::shared_ptr<FileOptions>& opt) {
        MmapFile* mmap_file = MmapFile::NewMmapFile(opt->file_name_, opt->flag_, opt->max_sz_);
        if (mmap_file == nullptr) {
            return nullptr;
        }
        SSTable* sstable = new SSTable(mmap_file);
        return sstable;
    }

    RC Bytes(uint64_t offset, uint64_t size, char* mmap_addr) {
        return file_->Bytes(offset, size, mmap_addr);
    }
    
    std::unique_ptr<File> file_;
};
#endif