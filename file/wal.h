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
    WALFile(MmapFile* mmap_file): file_(mmap_file), offset_(0) {}
public:
    static WALFile* newWALFile(const std::shared_ptr<FileOptions>& opt) {
        MmapFile* mmap_file = MmapFile::NewMmapFile(opt->file_name_, opt->flag_, opt->max_sz_);
        if (mmap_file == nullptr) {
            return nullptr;
        }
        WALFile* wal_file = new WALFile(mmap_file);
        return wal_file;
    }

    RC close() {
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        file_->close();
    }

    RC write(Entry* entry) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        std::string codec_entry;
        const size_t codec_entry_size = walCodec(entry, codec_entry);
        RC result = file_->append(offset_, codec_entry);
        offset_ += codec_entry.size();
        return result;
    }

    uint64_t size() {
        return offset_;
    }

    std::unique_ptr<File> file_;
    uint64_t offset_;
    mutable std::shared_mutex mutex_;

};
#endif