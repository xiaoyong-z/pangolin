#ifndef TABLE_H
#define TABLE_H
#include "sstable.h"
#include "util.h"
#include "builder.h"
#include "cache.h"
class LevelsManager;
class Table {
private:
    Table(SSTable* sstable_file, uint32_t file_id): sstable_(sstable_file), fd_(file_id), crc_(0), refs_(0) {}
public:
    static Table* NewTable(const std::string& dir_name, uint32_t file_id, uint64_t sstable_max_sz) {
        std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
        file_opt->file_name_ = Util::filePathJoin(dir_name, file_id, SSTableConfig::filePostfix);

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
        return builder->flush(sstable_.get(), crc_);
    }

    RC get(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {
        std::string key2 = key.ToString();
        if (key.compare(sstable_->min_key_) < 0 || key.compare(sstable_->max_key_) > 0) {
            return RC::TABLE_EXCEED_MINMAX;
        }

        if (BloomFilter::contains(key.data(), *sstable_->bloom_filter_) == false) {
            return RC::TABLE_BLOOM_FILTER_NOT_CONTAIN;
        }

        std::unique_ptr<BlockOffsetsIterator> iterator(sstable_->newIterator());
        uint32_t block_index = iterator->find(key2);
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
        if (block_iterator->find(key2, entry) == false) {
            return RC::TABLE_KEY_NOT_FOUND_IN_BLOCK;   
        }
        entry.key_ = key;
        return RC::SUCCESS;
    }

    RC open() {
        sstable_->init(crc_, size_); 
        return RC::SUCCESS;
    }

    uint64_t getCacheBlockId(uint32_t block_index) {    
        return static_cast<uint64_t>(block_index) | static_cast<uint64_t>(fd_) << 32;
    }

    inline uint32_t getFD() const {
        return fd_;
    }

    inline uint32_t getCRC() const {
        assert(crc_ != 0);
        return crc_;
    }

    inline const uint64_t getSize() const {
        return size_;
    }

    inline std::string& getMinKey() {
        return sstable_->getMinKey();
    }

    inline std::string& getMaxKey() {
        return sstable_->getMaxKey();
    }

    void increaseRef() {
        refs_.fetch_add(1);
    }

    void decreaseRef() {
        refs_.fetch_sub(1);
    }


private:
    std::unique_ptr<SSTable> sstable_;
    uint32_t fd_;
    uint32_t crc_;
    uint64_t size_;
    std::atomic<uint32_t> refs_;
};
#endif