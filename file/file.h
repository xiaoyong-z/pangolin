#ifndef FILE_H
#define FILE_H
#include <string>
#include "util.h"
class FileReader {
public:
    virtual int Read(char* buf) = 0;
    virtual void init(char* data) = 0;
};

struct FileOptions {
    uint32_t FID_;
    std::string file_name_;
    std::string dir_;
    std::string path_;
    int flag_;
    int max_sz_;
};

class File {
public:
    
    virtual RC truncate(uint64_t n) = 0;
    virtual RC sync() = 0;
    virtual RC rename(std::string string) = 0;
    virtual RC close() = 0;
    virtual RC fdelete() = 0;
    virtual RC NewReader(const std::shared_ptr<FileReader>& reader) = 0;
	virtual RC AllocateSlice(uint64_t size, uint64_t offset, char*& free_addr) = 0;
    virtual RC Bytes(uint64_t offset, int64_t size, char*& mmap_addr) = 0;
    virtual RC get_mmap_ptr(char*& mmap_data) = 0;
    
	// Slice(offset int) []byte
};
#endif