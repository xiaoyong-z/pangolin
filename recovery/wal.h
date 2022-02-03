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
#include "skipLists.h"
class SkipList;
class WALFile {
private:
    WALFile(MmapFile* mmap_file): file_(mmap_file), offset_(0) {}
public:
    static WALFile* newWALFile(const std::shared_ptr<FileOptions>& opt) {
        MmapFile* mmap_file = MmapFile::newMmapFile(opt->file_name_, opt->flag_, opt->max_sz_);
        if (mmap_file == nullptr) {
            return nullptr;
        }
        WALFile* wal = new WALFile(mmap_file);
        return wal;
    }

    RC close() {
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        std::string filename;
        RC rc = file_->getFilename(filename);
        assert(rc == RC::SUCCESS);
        rc = file_->close();
        assert(rc == RC::SUCCESS);
        assert(remove(filename.data()) == 0);
        return RC::SUCCESS;
    }

    RC write(Entry* entry) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        if (file_ == nullptr) {
            return RC::WAL_UNINTIALIZE;
        }
        std::string codec_entry;
        const size_t codec_entry_size = walCodec(entry, codec_entry);
        RC result = file_->append(offset_, codec_entry);
        offset_ += codec_entry_size;
        return result;
    }

    uint64_t size() {
        return offset_;
    }

    RC iterator(bool read_only, uint64_t start_offset, SkipList* skip_list, void (*func)(SkipList* skip_list, Entry* entry));

    RC decodeEntry(const std::shared_ptr<MmapFile::MmapReader>& reader, Entry* entry) {
        char* buf, *buf_start;
        uint64_t n = reader->read(16, buf);
        buf_start = buf;
        if (n != 16) {
            return RC::WAL_END_OF_FILE_ERROR;
        }
        uint32_t key_len = decodeFix32(buf);
        uint32_t value_len = decodeFix32(buf + 4);
        n = reader->read(key_len + value_len + CRC_SIZE_LEN, buf);
        if (n != key_len + value_len + CRC_SIZE_LEN) {
            return RC::WAL_END_OF_FILE_ERROR;
        }
        entry->key_.reset(buf, key_len);
        entry->value_.reset(buf + key_len, value_len);
        uint32_t crc = crc32c::Value(buf_start, 16 + key_len + value_len);
        if (crc != decodeFix32(buf + key_len + value_len)) {
            return RC::WAL_END_OF_FILE_ERROR;
        }
        return RC::SUCCESS;
    }

    std::unique_ptr<MmapFile> file_;
    uint64_t offset_;
    mutable std::shared_mutex mutex_;

};
#endif