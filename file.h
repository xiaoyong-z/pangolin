#include <string>

class FileReader {
public:
    virtual int Read(char* buf) = 0;
    virtual void init(char* data) = 0;
};

class File {
public:
    
    virtual bool truncate(uint64_t n) = 0;
    virtual bool sync() = 0;
    virtual bool rename(std::string string) = 0;
    virtual void close() = 0;
    virtual void fdelete() = 0;
    virtual bool NewReader(FileReader& reader) = 0;
    


    
    // NewReader(offset int) io.Reader
	// Bytes(off, sz int) ([]byte, error)
	// AllocateSlice(sz, offset int) ([]byte, int, error)
	// Slice(offset int) []byte
};