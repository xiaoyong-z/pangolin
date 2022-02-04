#ifndef UTIL_H
#define UTIL_H

#include <cassert>
#include <shared_mutex>
#include "rc.h"
#include "options.h"
#include "slice.h"
#include "coding.h"
#include "crc32c.h"


#define SSTABLE_SIZE_LEN 8
#define CRC_SIZE_LEN 4
#define META_SIZE 4
#define KEY_VALUE_LEN 16
#define LOG printf
#define ALIGN_NUM 8



using RWLock = std::shared_mutex;
using WriteLock = std::unique_lock<RWLock>;
using ReadLock = std::shared_lock<RWLock>;

enum ValueType { kTypeDeletion = 0x0, kTypeValue = 0x1 };

class Util {
public:
    static char* align(char* ptr) {
        return reinterpret_cast<char*>(align(reinterpret_cast<uint64_t>(ptr)));
    }

    static uint64_t align(uint64_t size) {
        return (size / ALIGN_NUM) * ALIGN_NUM + ((size % ALIGN_NUM == 0) ? 0 : ALIGN_NUM);
    }

    static inline std::string filePathJoin(const std::string& path, uint32_t fid, const std::string& postfix) {
        return path + "/" + std::to_string(fid) + "." + postfix;
    }

    static void encodeVersionNumType(char* addr, uint32_t version_num, ValueType value_type) {
        encodeFix32(addr, (version_num << 8) | (value_type & 0x0000000ff));
    }

    static void encodeVersionNum(std::string* key, uint32_t version_num) {
        encodeFix32(key, (version_num << 8));
    }

    static void decodeVersionNumType(char* addr, uint32_t& version_num, ValueType& value_type) {
        uint32_t temp = decodeFix32(addr);
        version_num = temp >> 8;
        value_type = static_cast<ValueType>(temp & 0x000000ff);
    }

    static int compareKey(const std::string& strA, const std::string& strB) {
        return compare(strA.data(), strB.data(), strA.size() - META_SIZE, strB.size() - META_SIZE);
    }

    static int compareKey(const Slice& sliceA, const Slice& sliceB) {
        return compare(sliceA.data(), sliceB.data(), sliceA.size() - META_SIZE, sliceB.size() - META_SIZE);
    }

    static int compareBytes(const std::string& strA, const std::string& strB) {
        int cmp = compare(strA.data(), strB.data(), strA.size() - META_SIZE, strB.size() - META_SIZE);
        if (cmp < 0) {
            return -1;
        } else if (cmp > 0) {
            return 1;
        } else {
            uint32_t versionNumTypeA = decodeFix32(strA.data() + strA.size() - META_SIZE);
            uint32_t versionNumTypeB = decodeFix32(strB.data() + strB.size() - META_SIZE);
            if (versionNumTypeA < versionNumTypeB) {
                return -1;
            } else if (versionNumTypeA > versionNumTypeB) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    static int compareBytes(const Slice& sliceA, const Slice& sliceB) {
        int cmp = compare(sliceA.data(), sliceB.data(), sliceA.size() - META_SIZE, sliceB.size() - META_SIZE);
        if (cmp < 0) {
            return -1;
        } else if (cmp > 0) {
            return 1;
        } else {
            uint32_t versionNumTypeA = decodeFix32(sliceA.data() + sliceA.size() - META_SIZE);
            uint32_t versionNumTypeB = decodeFix32(sliceB.data() + sliceB.size() - META_SIZE);
            if (versionNumTypeA < versionNumTypeB) {
                return 1;
            } else if (versionNumTypeA > versionNumTypeB) {
                return -1;
            } else {
                return 0;
            }
        }
    }

    static int compare(const char* charA, const char* charB, int sizeA, int sizeB) {
        const size_t min_len = (sizeA < sizeB) ? sizeA : sizeB;
        int r = memcmp(charA, charB, min_len);
        if (r == 0) {
            if (sizeA < sizeB)
                r = -1;
            else if (sizeA > sizeB)
                r = +1;
        }
        return r;
    }
};

namespace ManifestConfig {
    static const int DeleteRewriteThreshold = 10000;
    static const int DeleteRewriteRatio = 10;
    static const int manifestHeadSize = 8;
    static const int changeHeadSize = 8;
    static const std::string magicNum = "4521";
    static const std::string versionNum = "0001";
    static const std::string fileName = "MANIFEST";
    static const std::string rfileName = "REMANIFEST";
};

namespace SSTableConfig {
    static const std::string filePostfix = "sstable";
}

namespace WALConfig {
    static const std::string filePostfix = "wal";
}

namespace CompactionConfig {
    static const int compaction_duration = 1;
    static const int level0CompactionThreadshold = 10;
}

#endif
