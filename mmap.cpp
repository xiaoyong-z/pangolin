#include <sys/mman.h>
#include "util.h"
class MmapUtil {
public:
    static const MmapUtil& SingleInstance() {
        static MmapUtil mmap_instance;
        return mmap_instance;
    }

    char* Mmap(int fd, bool writable, uint64_t size) const {
        int prots = PROT_READ;
        if (writable) {
            prots |= PROT_WRITE;
        }
        return (char*)mmap(0, size, prots, MAP_SHARED, fd, 0);
    }

    bool Msync(char* mmap_data, uint64_t size) const {
        if (msync(mmap_data, size, MS_SYNC) == -1) {
            LOG("msync error");
            return false;
        }
        return true;
    }

    bool Munmap(char* mmap_data, uint64_t size) const {
        if (munmap(mmap_data, size) == -1) {
            LOG("munmap error");
            return false;
        }
        return true;
    }

    bool Mremap(char* mmap_data, uint64_t old_size, uint64_t new_size) const {
        if (mremap(mmap_data, old_size, new_size, MREMAP_MAYMOVE)) {

        }
    }
};