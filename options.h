#ifndef OPTIONS_H
#define OPTIONS_H
#include <string>
struct Options {
	Options(const std::string& work_dir, uint64_t mem_table_size, uint64_t ssTable_max_sz,
		int block_size, double bloom_false_positive) : work_dir_(work_dir), mem_table_size_(mem_table_size),
		ssTable_max_sz_(ssTable_max_sz), block_size_(block_size), bloom_false_positive_(bloom_false_positive) {}
	std::string work_dir_;
	uint64_t mem_table_size_;
	uint64_t ssTable_max_sz_;
	// BlockSize is the size of each block inside SSTable in bytes.
	int block_size_;
	// BloomFalsePositive is the false positive probabiltiy of bloom filter.
	double bloom_false_positive_;
};

#endif