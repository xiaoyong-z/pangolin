#ifndef TABLE_H
#define TABLE_H
#include "sstable.h"
#include "util.h"
#include "builder.h"
#include "cache.h"
#include "iterator.h"
#include "kv.pb.h"
class LevelsManager;
class SSTable;
class Builder;
class Table {
private:
    Table(SSTable* sstable_file, uint32_t file_id);
public:
    static Table* NewTable(const std::string& dir_name, uint32_t file_id, uint64_t sstable_max_sz);

    RC flush(const std::shared_ptr<Builder>& builder, bool sync = true);

    static RC get(std::shared_ptr<Table>& table, const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt);

    RC open();

    uint64_t getCacheBlockId(uint32_t block_index);

    uint32_t getFD();

    uint32_t getCRC();

    uint64_t getSize();

    uint32_t getBlockCount();

    std::string& getMinKey();

    std::string& getMaxKey();

    int getMaxVersion();

    void increaseRef();

    void decreaseRef();

    std::shared_ptr<SSTable>& getSSTable();

    const pb::IndexBlock& getIndexBlock();

    RC sync();

private:
    std::shared_ptr<SSTable> sstable_;
    uint32_t fd_;
    uint32_t crc_;
    std::atomic<uint32_t> refs_;
};
#endif