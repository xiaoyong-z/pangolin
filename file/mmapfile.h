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
   public:
    ~MmapFile();
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
            memmove(buf, data_ + offset_, n);
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
        if (flag == O_RDONLY) {
            writable = false;
        }
        struct stat f_stat;
        if (fstat(fd_, &f_stat) == -1) {
            LOG("unable to fstat: %s", filename.c_str());
            close();
            return RC::MMAPFILE_STAT;
        }
        int file_size = f_stat.st_size;
        if (file_size == 0) {
            if (ftruncate(fd_, max_sz) == -1) {
                LOG("unable to truncate: %s", filename.c_str());
                close();
                return RC::MMAPFILE_TRUNCATE;
            }
            file_size = max_sz;
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
        if (mmap_data_ == nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return RC::MMAPFILE_MMAP_UNINITIALIZE;
        }
        reader->init(mmap_data_);
        return RC::SUCCESS;
    }

    RC bytes(uint64_t offset, int64_t size, char*& mmap_addr) {
        if (mmap_data_ == nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return RC::MMAPFILE_MMAP_UNINITIALIZE;
        }
        if (offset + size > map_size_) {
            LOG("mmap doesn't have enough space : %s", filename_.c_str());
            return RC::MMAPFILE_NOT_ENOUGH_SPACE;
        }
        mmap_addr = mmap_data_ + offset;
        return RC::SUCCESS;
    }

    RC rename(std::string string) {
        assert(false);
        return RC::SUCCESS;
    }

    RC append(uint64_t offset, std::string str) {
        if (mmap_data_ == nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return RC::MMAPFILE_MMAP_UNINITIALIZE;
        }
        uint64_t size = str.size();
        if (offset + size > map_size_) {
            const uint64_t oneGB = 1 << 30;
            uint64_t growBy = map_size_;
            growBy = std::min(growBy, oneGB);
            growBy = std::max(growBy, size);
            RC result = truncate(growBy + map_size_);
            if (result != RC::SUCCESS) {
                return result;
            }
        }
        memmove(mmap_data_ + offset, str.data(), str.size());
        return RC::SUCCESS;
    }

    RC AllocateSlice(uint64_t size, uint64_t offset, char*& free_addr) {
        if (mmap_data_ == nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return RC::MMAPFILE_MMAP_UNINITIALIZE;
        }
        uint64_t start = offset + 8;
        if (start + size > map_size_) {
            const uint64_t oneGB = 1 << 30;
            uint64_t growBy = map_size_;
            growBy = std::min(growBy, oneGB);
            growBy = std::max(growBy, size + 8);
            RC result = truncate(growBy + map_size_);
            if (result != RC::SUCCESS) {
                return result;
            }
        }
        uint64_t *ptr = reinterpret_cast<uint64_t*>(mmap_data_);
        ptr[offset] = size;
        free_addr = reinterpret_cast<char*>(ptr + start);
        // *(uint64_t*)mmap_data_[start] = size;
        return RC::SUCCESS;
    } 

    RC get_mmap_ptr(char*& mmap_data) {
        if (mmap_data_ == nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return RC::MMAPFILE_MMAP_UNINITIALIZE;
        }
        mmap_data = mmap_data_;
        return RC::SUCCESS;
    }


   private:
    char* mmap_data_;
    int fd_;
    uint64_t map_size_;
    std::string filename_;
};
#endif