#ifndef ITERATOR_H
#define ITERATOR_H
#include "util.h"
#include "skipLists.h"
#include "block.h"
#include "table.h"
class Iterator {
public:
    virtual ~Iterator() {};
    virtual void Next() = 0;
    virtual bool Valid() = 0;
    virtual void Close() = 0;
    virtual void Rewind() = 0;
    virtual bool Seek(const std::string& key) = 0;
    virtual std::string getKey() = 0;
    virtual std::string getValue() = 0;
    virtual void getEntry(Entry& entry) = 0;
};

class MemTable;
class SkipNode;
class SkipList;
class Table;

class SkipListIterator: public Iterator {
public:
    SkipListIterator(const std::shared_ptr<SkipList>& skiplist);
    static std::shared_ptr<SkipListIterator> NewIterator(const std::shared_ptr<SkipList>& skiplist);
    ~SkipListIterator() override;
    void Next() override;
    bool Valid() override;
    void Close() override;
    void Rewind() override;
    bool Seek(const std::string& key);
    std::string getKey() override;
    std::string getValue() override;
    void getEntry(Entry& entry) override;

private:
    SkipNode* it_;
    std::shared_ptr<SkipList> skip_list_;
};

class BlockIterator: public Iterator {
public:
    BlockIterator(const std::shared_ptr<Block>& block);
    static std::shared_ptr<BlockIterator> NewIterator(const std::shared_ptr<Block>& block);
    ~BlockIterator();
    void Close() override;
    void Rewind() override;
    bool Seek(const std::string& key) override;
    bool Valid() override;
    void Next() override;
    std::string getKey() override;
    std::string getValue() override;
    void getEntry(Entry& entry) override;

private:
    const std::string getKey(uint32_t index);
    const std::string getValue(uint32_t index);

    std::shared_ptr<Block> block_;
    const std::string& base_key_;
    const std::vector<uint32_t>& offset_;
    const std::string& content_;
    uint32_t pos_;
    uint32_t end_;
    int keys_len_;
};

class TableIterator: public Iterator {
public:
    TableIterator(const std::shared_ptr<Table>& table);
    static std::shared_ptr<TableIterator> NewIterator(const std::shared_ptr<Table>& table);
    ~TableIterator();
    void Close() override;
    void Rewind() override;
    bool Seek(const std::string& key) override;
    bool Valid() override;
    void Next() override;
    std::string getKey() override;
    std::string getValue() override;
    void getEntry(Entry& entry) override;

private:
    const pb::BlockOffset& getBlockOffset(uint32_t index);

    const uint32_t getBlockIndex(const std::string& key);

    inline void updateBlock();

    void updateBlock(uint32_t pos);

    std::shared_ptr<Table> table_;
    uint32_t pos_;
    uint32_t end_;
    const pb::IndexBlock& index_block_; 
    std::shared_ptr<BlockIterator> block_iterator_;
};

// class TableIterator: public Iterator {

// };
#endif 