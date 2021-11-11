#ifndef WAL_H
#define WAL_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include "file.h"
#include "mmapfile.h"
#include "entry.h"
#include "util.h"
class WALFile {
public:
    WALFile(const FileOptions& opt) {
        MmapFile* mmap_file = MmapFile::NewMmapFile(opt.file_name_, opt.flag_, opt.max_sz_);
        if (mmap_file == nullptr) {
            file_ = nullptr;
        } else {
            file_.reset(mmap_file);
        }
    }

    RC Close() {
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        file_->close();
    }

    RC Write(const std::shared_ptr<strEntry>& entry) {
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        // TODO: WAL
    }


    std::unique_ptr<File> file_;
    mutable std::shared_mutex mutex_;
};
#endif 