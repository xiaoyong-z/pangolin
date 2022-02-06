#ifndef OPTIONS_H
#define OPTIONS_H
#include <string>
struct Options {
	Options(const std::string& work_dir, uint64_t mem_table_size, uint64_t SSTable_max_sz,
		uint64_t block_size, double bloom_false_positive, int max_level_num = 5, int level_size_multiplier = 10) : 
		work_dir_(work_dir), mem_table_size_(mem_table_size), SSTable_max_sz(SSTable_max_sz), 
		block_size_(block_size), bloom_false_positive_(bloom_false_positive), max_level_num_(max_level_num),
		level_size_multiplier_(level_size_multiplier){} 

	inline const int getCompactionThreadNum() const {
		return max_level_num_;
	}

	inline const int getMaxLevelNum() const {
		return max_level_num_;
	}

	inline const uint64_t getSSTableSize() const {
		return SSTable_max_sz;
	}

	inline const int getLevelSizeMultiplier() const {
		return level_size_multiplier_;
	}


	std::string work_dir_;
	uint64_t mem_table_size_;
	uint64_t SSTable_max_sz;
	// BlockSize is the size of each block inside SSTable in bytes.
	uint64_t block_size_;
	// BloomFalsePositive is the false positive probabiltiy of bloom filter.
	double bloom_false_positive_;


	int max_level_num_;
	int level_size_multiplier_;
};

#endif