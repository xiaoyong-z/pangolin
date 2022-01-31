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
    Block();

    Block(const pb::BlockOffset& blockOffset, SSTable* sstable);

    int diffKey(const Slice& key);

    RC insert(const Entry& entry);

    bool checkFinish(const Entry& entry, int max_size);

    uint32_t getKeyCount();

    void finish();

    std::string& getBaseKey();

    std::string& getContent();

    std::vector<uint32_t>& getOffsets();

    uint32_t getSize();

private:
    std::string content_;
    std::vector<uint32_t> offset_;

    std::string base_key_;
    uint32_t size_;
};
#endif