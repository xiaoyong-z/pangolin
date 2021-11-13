#ifndef INDEXBLOCK_H
#define INDEXBLOCK_H
#include <string>
#include <vector>
struct BlockOffSet {
    std::string base_key;
    uint32_t offset;
    uint32_t len;
};

struct IndexBlock {
    std::vector<BlockOffSet> offsets_;
    std::string bloom_filter_;
    uint32_t max_version_;
    uint32_t key_count_;
};
#endif