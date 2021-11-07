#ifndef WAL_H
#define WAL_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include "file.h"
#include "mmapfile.h"
#include "entry.h"
class WALFile {
public:
    WALFile(const FileOptions& opt) {
        file_ = std::make_unique<MmapFile>(opt.file_name_, opt.flag_, opt.max_sz_);
    }

    void Close() {
        file_->close();
    }

    void Write(const std::shared_ptr<strEntry>& entry) {
        // TODO: WAL
    }


    std::unique_ptr<File> file_;
    mutable std::shared_mutex mutex_;
};
#endif 