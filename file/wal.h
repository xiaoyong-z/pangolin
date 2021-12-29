#ifndef WAL_H
#define WAL_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include "file.h"
#include "mmapfile.h"
#include "entry.h"
#include "util.h"
#include "codec.h"

class WALFile {
private:
    WALFile(MmapFile* mmap_file): file_(mmap_file) {}
public:
    static WALFile* NewWALFile(const std::shared_ptr<FileOptions>& opt) {
        MmapFile* mmap_file = MmapFile::NewMmapFile(opt->file_name_, opt->flag_, opt->max_sz_);
        if (mmap_file == nullptr) {
            return nullptr;
        }
        WALFile* wal_file = new WALFile(mmap_file);
        return wal_file;
    }

    RC Close() {
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        file_->close();
    }

    RC Write(Entry* entry) {
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        const char* walData = WalCodec(entry);
        std::unique_lock<std::shared_mutex> lock(mutex_);
        char* addr;
        RC result = file_->AllocateSlice(strlen(walData), 0, addr);
        if (result != RC::SUCCESS) {
            return result;
        }
        memmove(addr, walData, strlen(walData));
        return RC::SUCCESS;
    }


    std::unique_ptr<File> file_;
    mutable std::shared_mutex mutex_;
};
#endif