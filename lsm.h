#ifndef LSM_H
#define LSM_H
#include <string>
#include "memtable.h"
#include "levels.h"
#include "file.h"
struct Options {
	std::string work_dir_;
	uint64_t mem_table_size_;
	uint64_t ssTable_max_sz_;
	// BlockSize is the size of each block inside SSTable in bytes.
	int block_size_;
	// BloomFalsePositive is the false positive probabiltiy of bloom filter.
	double bloom_false_positive_;
};

class LSM {

public:
	RC Close() {
		RC result;
		// Todo 
		// if (memtable_ != nullptr) {
		// 	result = memtable_->close();
		// }
		return result;
	}
	
	RC Get() {
		return RC::SUCCESS;
	}


private:
	std::shared_ptr<MemTable> memtable_;
	std::vector<std::shared_ptr<MemTable>> immutables_;
	std::unique_ptr<LevelManager> level_mangaer_;
	std::shared_ptr<FileOptions> options;


};
#endif