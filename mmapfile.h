#ifndef MMAPFILE_H
#define MMAPFILE_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <cstring>
#include <cassert>

#include "mmap.h"
#include "util.h"
#include "file.h"
class MmapFile: public File{
   private:
    MmapFile() {}
   public:
    class Reader: public FileReader{
    public:
        Reader(){}

        void init(char* data) {
            data_ = data;
            offset_ = 0;
        }

        int Read(char* buf) {
            int data_len = strlen(data_);
            if (offset_ >= data_len) {
                return 0;
            }
            int buf_size = strlen(buf);
            int n = std::min(buf_size, data_len - offset_);
            memcpy(buf, data_ + offset_, n);
            offset_ += n;
            return n;
        }
    private:
        char* data_;
        int offset_;
    };

    static MmapFile* NewMmapFile(std::string filename, int flag, uint64_t max_sz) {
        MmapFile* mmap_file = new MmapFile();
        RC result = mmap_file->open(filename, flag, max_sz);
        if (result != RC::SUCCESS) {
            return nullptr;
        }
        return mmap_file;
    }

    RC open(std::string filename, int flag, uint64_t max_sz) {
        fd_ = ::open(filename.c_str(), flag, 0666);
        if (fd_ < 0) {
            LOG("unable to open: %s", filename.c_str());
            close();
            return RC::MMAPFILE_OPEN;
        }
        filename_ = filename;
        bool writable = true;
        if (flag & O_RDONLY) {
            writable = false;
        }
        struct stat f_stat;
        if (fstat(fd_, &f_stat) == -1) {
            LOG("unable to fstat: %s", filename.c_str());
            close();
            return RC::MMAPFILE_STAT;
        }
        uint64_t f_size = f_stat.st_size;
        if (f_size < max_sz) {
            if (ftruncate(fd_, max_sz) == -1) {
                LOG("unable to truncate: %s", filename.c_str());
                close();
                return RC::MMAPFILE_TRUNCATE;
            }
            f_size = max_sz;
        }
        
        char* addr;
        RC result = MmapUtil::SingleInstance().Mmap(fd_, writable, max_sz, addr);
        if (result == RC::MMAP_MMAP) {
            LOG("unable to mmap: %s", filename.c_str());
            close();
            return result;
        }
        map_size_ = max_sz;
        mmap_data_ = addr;
        return RC::SUCCESS;
    }

    RC close() {
        if (fd_ < 0) {
            return RC::SUCCESS;
        }
        if (mmap_data_ != nullptr) {
            RC result = MmapUtil::SingleInstance().Munmap(mmap_data_, map_size_);
            if (result != RC::SUCCESS) {
                LOG("mmump failed: %s", filename_.c_str());
                return result;
            }
        }
        if (::close(fd_) == -1) {
            LOG("unable to close: %s", filename_.c_str());
            return RC::MMAPFILE_CLOSE; 
        }
        return RC::SUCCESS;
    }

    RC fdelete() {
        if (fd_ >= 0) {
            if (ftruncate(fd_, 0) == -1) {
                LOG("unable to fstat: %s", filename_.c_str());
                return RC::MMAPFILE_STAT;
            }
            close();
            if (remove(filename_.c_str()) == -1) {
                LOG("remove error");
                return RC::MMAPFILE_REMOVE;
            }
        }
        return RC::SUCCESS;
    }

    RC sync() {
        if (mmap_data_ == nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return RC::MMAPFILE_MMAP_UNINITIALIZE;
        }
        return MmapUtil::SingleInstance().Msync(mmap_data_, map_size_);
    }

    RC truncate(uint64_t size) {
        if (size >= 0) {
            if (ftruncate(fd_, size) == -1) {
                LOG("unable to truncate: %s", filename_.c_str());
                return RC::MMAPFILE_TRUNCATE;
            }
            if (size > map_size_) {
                char* addr;
                RC result = MmapUtil::SingleInstance().Mremap(mmap_data_, map_size_, size, addr);
                if (result != RC::SUCCESS) {
                    LOG("remmap failed: %s", filename_.c_str());
                    return result;
                }
                mmap_data_ = addr;
                map_size_ = size;
            }
        }
        return RC::SUCCESS;
    }

    RC NewReader(const std::shared_ptr<FileReader>& reader) {
        if (mmap_data_ = nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return RC::MMAPFILE_MMAP_UNINITIALIZE;
        }
        reader->init(mmap_data_);
        return RC::SUCCESS;
    }

    RC rename(std::string string) {
        assert(false);
        return RC::SUCCESS;
    }


   private:
    char* mmap_data_;
    int fd_;
    uint64_t map_size_;
    std::string filename_;
};
#endif