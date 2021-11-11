#ifndef MMAP_H
#define MMAP_H

#include <sys/mman.h>
#include "util.h"
class MmapUtil {
public:
    static const MmapUtil& SingleInstance() {
        static MmapUtil mmap_instance;
        return mmap_instance;
    }

    RC Mmap(int fd, bool writable, uint64_t size, char*& mmap_data) const {
        int prots = PROT_READ;
        if (writable) {
            prots |= PROT_WRITE;
        }
        char* addr = (char*)mmap(0, size, prots, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            return RC::MMAP_MMAP;
        }
        mmap_data = addr;
        return RC::SUCCESS;
    }

    RC Msync(char* mmap_data, uint64_t size) const {
        if (msync(mmap_data, size, MS_SYNC) == -1) {
            LOG("msync error");
            return RC::MMAP_MSYNC;
        }
        return RC::SUCCESS;
    }

    RC Munmap(char* mmap_data, uint64_t size) const {
        if (munmap(mmap_data, size) == -1) {
            LOG("munmap error");
            return RC::MMAP_MUNMAP;
        }
        return RC::SUCCESS;
    }

    RC Mremap(char* mmap_data, uint64_t old_size, uint64_t new_size, char*& new_mmap_data) const {
        char* addr = (char*)mremap(mmap_data, old_size, new_size, MREMAP_MAYMOVE);
        if (addr == MAP_FAILED) {
            return RC::MMAP_MREMAP;
        }
        return RC::SUCCESS;
    }
};
#endif