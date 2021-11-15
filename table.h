#ifndef TABLE_H
#define TABLE_H
#include "sstable.h"
#include "util.h"
#include "builder.h"
class Table {
private:
    Table(SSTable* sstable_file): sstable_(sstable_file) {}
public:
    static Table* NewTable(const std::string& dir_name, uint64_t file_id, uint64_t sstable_max_sz) {
        std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
        file_opt->file_name_ = dir_name + std::to_string(file_id);
        file_opt->dir_ = dir_name;
        file_opt->flag_ = O_CREAT | O_RDWR;
        file_opt->max_sz_ = sstable_max_sz;
        SSTable* sstable_file = SSTable::NewSSTableFile(file_opt);
        if (sstable_file == nullptr) {
            return nullptr;
        }
        Table* table = new Table(sstable_file);
        return table;
    }

    RC flush(Builder& builder) {
        return builder.flush(sstable_.get());
    }

    RC open() {

    }
    
    std::unique_ptr<SSTable> sstable_;
};
#endif