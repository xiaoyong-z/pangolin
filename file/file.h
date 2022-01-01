#ifndef FILE_H
#define FILE_H
#include <string>
#include <memory>
#include "util.h"
class FileReader {
public:
    virtual ~FileReader() {};
    virtual uint64_t read(char* buf) = 0;
    virtual uint64_t read(uint64_t n, char*& buf) = 0;
    // virtual void init(char* data, uint64_t map_size) = 0;
};

struct FileOptions {
    FileOptions() {}
    FileOptions(uint32_t fid, const std::string& dir, int flag, int max_sz): 
        FID_(fid), dir_(dir), flag_(flag), max_sz_(max_sz) {}
    uint32_t FID_;
    std::string file_name_;
    std::string dir_;
    int flag_;
    int max_sz_;
};

class File {
public:
    virtual ~File() {};
    virtual RC truncate(uint64_t n) = 0;
    virtual RC sync() = 0;
    virtual RC rename(std::string string) = 0;
    virtual RC close() = 0;
    virtual RC fdelete() = 0;
    // virtual RC newReader(const std::shared_ptr<FileReader>& reader) = 0;
	virtual RC allocateSlice(uint64_t size, uint64_t offset, char*& free_addr) = 0;
    virtual RC bytes(uint64_t offset, int64_t size, char*& mmap_addr) = 0;
    virtual RC get_mmap_ptr(char*& mmap_data) = 0;
    virtual RC append(uint64_t offset, std::string str) = 0;
    
	// Slice(offset int) []byte
};
#endif