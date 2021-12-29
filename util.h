#ifndef UTIL_H
#define UTIL_H

#include <cassert>
#include "rc.h"
#include "options.h"
#include "slice.h"
#include "coding.h"
#include "crc32c.h"


#define SSTABLE_SIZE_LEN 8
#define CRC_SIZE_LEN 4
#define LOG printf
#define ALIGN_NUM 8

class Util {
public:
    static char* align(char* ptr) {
        return reinterpret_cast<char*>(align(reinterpret_cast<uint64_t>(ptr)));
    }

    static uint64_t align(uint64_t size) {
        return (size / ALIGN_NUM) * ALIGN_NUM + ((size % ALIGN_NUM == 0) ? 0 : ALIGN_NUM);
    }

    static inline std::string filePathJoin(const std::string& path, uint64_t fid, const std::string& postfix) {
        return path + "/" + std::to_string(fid) + "." + postfix;
    }
};
#endif
