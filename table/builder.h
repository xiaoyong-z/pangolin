#ifndef BUILDER_H
#define BUILDER_H
#include <vector>
#include "file.h"
#include "block.h"
#include "util.h"
#include "bloomFilter.h"
#include "sstable.h"
#include "kv.pb.h"
class Block;
class SSTable;

class Builder {
public:
    Builder(const std::shared_ptr<Options>& opt);

    RC insert(const Entry& entry);

    RC flush(SSTable* sstable, uint32_t& table_crc32);

    RC indexBuilder(const std::string& filter, std::string& content, uint64_t& block_len);


private:
    std::shared_ptr<Options> opt_;
    std::shared_ptr<Block> cur_block_;
    std::vector<std::shared_ptr<Block>> blocks_;
    std::vector<uint32_t> key_hashs_;
    uint32_t key_count_;
    uint32_t max_version_;
};
#endif
