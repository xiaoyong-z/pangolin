#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <cstring>
#include <cassert>

#include "mmap.cpp"
#include "util.h"
#include "file.h"
class MmapFile: public File{
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

    MmapFile() : mmap_data_(nullptr), fd_(-1) {}

    bool open(std::string filename, int flag, uint64_t max_sz) {
        fd_ = ::open(filename.c_str(), flag, 0666);
        if (fd_ < 0) {
            LOG("unable to open: %s", filename.c_str());
            close();
            return false;
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
            return false;
        }
        uint64_t f_size = f_stat.st_size;
        if (f_size < max_sz) {
            if (ftruncate(fd_, max_sz) == -1) {
                LOG("unable to truncate: %s", filename.c_str());
                close();
                return false;
            }
            f_size = max_sz;
        }
        char* addr = MmapUtil::SingleInstance().Mmap(fd_, writable, max_sz);
        if (addr == MAP_FAILED) {
            LOG("unable to mmap: %s", filename.c_str());
            close();
            return false;
        }
        map_size_ = max_sz;
        mmap_data_ = addr;
        return true;
    }

    void close() {
        if (fd_ < 0) {
            return;
        }
        if (mmap_data_ != nullptr) {
            MmapUtil::SingleInstance().Munmap(mmap_data_, map_size_);
        }
        if (::close(fd_) == -1) {
            LOG("unable to close: %s", filename_.c_str());
        }
    }

    void fdelete() {
        if (fd_ >= 0) {
            if (ftruncate(fd_, 0) == -1) {
                LOG("unable to fstat: %s", filename_.c_str());
            }
            close();
            if (remove(filename_.c_str()) == -1) {
                LOG("remove error");
            }
        }
    }

    bool sync() {
        if (mmap_data_ == nullptr) {
            LOG("mmap is not initialized: %s", filename_.c_str());
            return false;
        }
        return MmapUtil::SingleInstance().Msync(mmap_data_, map_size_);
    }

    bool truncate(uint64_t size) {
        if (size >= 0) {
            if (ftruncate(fd_, size) == -1) {
                LOG("unable to truncate: %s", filename_.c_str());
                return false;
            }
            if (size > map_size_) {
                char* addr = MmapUtil::SingleInstance().Mremap(mmap_data_,
                                                               map_size_, size);
                if (addr == MAP_FAILED) {
                    LOG("remmap failed: %s", filename_.c_str());
                    return false;
                }
                mmap_data_ = addr;
                map_size_ = size;
            }
        }
        return true;
    }

    bool NewReader(Reader &reader) {
        if (mmap_data_ = nullptr) {
            return false;
        }
        reader.init(mmap_data_);
        return true;
    }

    bool rename(std::string string) {
        assert(false);
        return false;
    }


   private:
    char* mmap_data_;
    int fd_;
    uint64_t map_size_;
    std::string filename_;
};

int main() {}