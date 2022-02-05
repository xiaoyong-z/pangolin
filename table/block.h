#ifndef BLOCK_H
#define BLOCK_H
#include <vector>
#include "coding.h"
#include "crc32c.h"
#include "sstable.h"
#include "util.h"
#include "kv.pb.h"
#include "sstable.h"
class SSTable;
class Block {
public:
    friend class Builder;
    Block(uint64_t max_size);

    Block(const pb::BlockOffset& blockOffset, SSTable* sstable, uint64_t max_size = 0);

    int diffKey(const Slice& key);

    RC insert(const Entry& entry);

    bool checkFinish(const Entry& entry);

    uint64_t estimateSize();

    uint64_t estimateSize(const Entry& entry);

    uint32_t getKeyCount();

    void finish();

    std::string& getBaseKey();

    std::string& getContent();

    std::vector<uint32_t>& getOffsets();

    uint64_t getSize();

private:
    std::string content_;
    std::vector<uint32_t> offset_;

    std::string base_key_;
    uint64_t size_;
    uint64_t max_size_;
};
#endif