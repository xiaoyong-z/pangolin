#include "skipLists.h"

double calculateKeyScore(const Slice& key) {
    size_t len = key.size(); 
    const char* data = key.data();
    if (len > 8) {
        len = 8;
    }
    uint64_t hash = 0;
    for (size_t i = 0; i < len; i++) {
        uint64_t shift = uint64_t(64 - 8 - i * 8);
        hash |= uint64_t(data[i]) << shift;
    }
    return double(hash);
}