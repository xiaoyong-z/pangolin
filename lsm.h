#ifndef LSM_H
#define LSM_H
#include <string>
struct Options {
	std::string work_dir_;
	uint64_t mem_table_size_;
	uint64_t ssTable_max_sz_;
	// BlockSize is the size of each block inside SSTable in bytes.
	int block_size_;
	// BloomFalsePositive is the false positive probabiltiy of bloom filter.
	double bloom_false_positive;
};
#endif