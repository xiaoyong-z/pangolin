#ifndef BLOCK_H
#define BLOCK_H
#include <vector>
class Block {
public:
    RC insert(strEntry* entry) {
        
    }
private:

    std::string last_key_;
    std::vector<int> offset_;
    int checksum_;
};
#endif