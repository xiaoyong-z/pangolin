#ifndef TABLE_H
#define TABLE_H
#include "sstable.h"
#include "util.h"
#include "builder.h"
#include "cache.h"
class LevelManager;
class Table {
private:
    Table(SSTable* sstable_file, uint32_t file_id): sstable_(sstable_file), fd_(file_id) {}
public:
    static Table* NewTable(const std::string& dir_name, uint32_t file_id, uint64_t sstable_max_sz) {
        std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
        file_opt->file_name_ = dir_name + std::to_string(file_id);
        file_opt->dir_ = dir_name;
        file_opt->flag_ = O_CREAT | O_RDWR;
        file_opt->max_sz_ = sstable_max_sz;
        SSTable* sstable_file = SSTable::newSSTableFile(file_opt);
        if (sstable_file == nullptr) {
            return nullptr;
        }
        Table* table = new Table(sstable_file, file_id);
        return table;
    }

    RC flush(const std::shared_ptr<Builder>& builder) {
        return builder->flush(sstable_.get());
    }

    RC get(const std::string& key, Entry& entry, const std::shared_ptr<Options>& opt) {
        if (key < sstable_->min_key_ || key > sstable_->max_key_) {
            return RC::TABLE_EXCEED_MINMAX;
        }

        if (BloomFilter::contains(key.data(), *sstable_->bloom_filter_) == false) {
            return RC::TABLE_BLOOM_FILTER_NOT_CONTAIN;
        }

        std::unique_ptr<BlockOffsetsIterator> iterator(sstable_->newIterator());
        uint32_t block_index = iterator->find(key);
        const pb::BlockOffset& blockOffset = iterator->getBlockOffset(block_index);
        uint64_t bId = getCacheBlockId(block_index);

        Block* block_ptr;
        Cache& cache = Cache::Instance();
        if (cache.getBlockCache(bId, block_ptr) == false) {
            Block block(blockOffset, sstable_.get());
            cache.setBlockCache(bId, std::move(block));
            cache.getBlockCache(bId, block_ptr);
        }
        
        std::unique_ptr<BlockIterator> block_iterator(block_ptr->newIterator());
        entry.value_ = block_iterator->find(key);
        if (entry.value_ == "") {
            return RC::TABLE_KEY_NOT_FOUND_IN_BLOCK;   
        }
        
        return RC::SUCCESS;
    }

    RC open() {
        sstable_->init();
        return RC::SUCCESS;
    }

    uint64_t getCacheBlockId(uint32_t block_index) {
        
        return block_index || static_cast<uint64_t>(fd_) << 32;
    }

private:
    std::unique_ptr<SSTable> sstable_;
    uint32_t fd_;
};
#endif