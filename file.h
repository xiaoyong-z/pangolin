#ifndef FILE_H
#define FILE_H
#include <string>

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
    
    virtual bool truncate(uint64_t n) = 0;
    virtual bool sync() = 0;
    virtual bool rename(std::string string) = 0;
    virtual void close() = 0;
    virtual void fdelete() = 0;
    virtual bool NewReader(const std::shared_ptr<FileReader>& reader) = 0;
    
    

    
    // NewReader(offset int) io.Reader
	// Bytes(off, sz int) ([]byte, error)
	// AllocateSlice(sz, offset int) ([]byte, int, error)
	// Slice(offset int) []byte
};
#endif