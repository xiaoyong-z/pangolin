#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "xxh32.hpp"
#include "coding.h"
#define ln2 0.69314718056
class BloomFilter {
   private:
    const static uint32_t seed = 0xbc9f1d34;
    BloomFilter() {}

   public:
    // keys has already been hashed
    static void createFilter(const std::vector<uint32_t>& keys,
                             std::vector<char>& filter, int bits_per_key) {
        if (bits_per_key < 0) {
            bits_per_key = 0;
        }
        char k = double(bits_per_key) * ln2;
        k = std::min((char)30, std::max((char)1, k));
        uint32_t nBits = std::max(keys.size() * bits_per_key, (size_t)64);
        uint32_t nBytes = (nBits + 7) / 8;
        nBits = nBytes * 8;

        filter.resize(nBytes + 1);
        for (auto h : keys) {
            uint32_t delta = h >> 17 | h << 15;
            for (int j = 0; j < k; j++) {
                uint32_t bitPos = h % (uint32_t)(nBits);
                filter[bitPos / 8] |= 1 << (bitPos % 8);
                h += delta;
            }
        }
        filter[nBytes] = char(k);
    }

    static bool Contains(const char* key, const std::vector<char>& filter) {
        if (filter.size() < 2) {
            return false;
        }
        uint32_t nBytes = filter.size() - 1;
        char k = filter[nBytes];
        uint32_t nBits = nBytes * 8;
        uint32_t h = Hash(key);
        uint32_t delta = h >> 17 | h << 15;
        for (int j = 0; j < k; j++) {
            uint32_t bitPos = h % (uint32_t)(nBits);
            if ((filter[bitPos / 8] & (1 << (bitPos % 8))) == 0) {
                return false;
            }
            h += delta;
        }
        return true;
    }

    static int calBitsPerKey(int numEntries, double fp) {
        int size = -1 * ((double)numEntries * std::log(fp)) /
                   (std::pow((double)ln2, (double)2));
        return (int)std::ceil((double)size / (double)numEntries);
    }

    static uint32_t Hash(const char* key) {
        //return xxh32::hash(key, strlen(key), seed);
        return LevelDBHash(key, strlen(key), seed);
    }

    static uint32_t LevelDBHash(const char* data, size_t n, uint32_t seed) {
        // Similar to murmur hash
        const uint32_t m = 0xc6a4a793;
        const uint32_t r = 24;
        const char* limit = data + n;
        uint32_t h = seed ^ (n * m);

        // Pick up four bytes at a time
        while (data + 4 <= limit) {
            uint32_t w = DecodeFix32(data);
            data += 4;
            h += w;
            h *= m;
            h ^= (h >> 16);
        }

        // Pick up remaining bytes
        switch (limit - data) {
            case 3:
                h += static_cast<uint8_t>(data[2]) << 16;
            case 2:
                h += static_cast<uint8_t>(data[1]) << 8;
            case 1:
                h += static_cast<uint8_t>(data[0]);
                h *= m;
                h ^= (h >> r);
                break;
        }
        return h;
    }
};
