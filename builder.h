#ifndef BUILDER_H
#define BUILDER_H
#include <vector>
#include "block.h"
class Builder {
public:
    RC insert(strEntry* entry) {
        if (cur_block == nullptr) {
            cur_block = std::make_shared<Block>();
        }
        cur_block->insert(entry);

        return RC::SUCCESS;
    }


private:
    std::shared_ptr<Block> cur_block;
    std::vector<std::shared_ptr<Block>> blocks_;
    int offset;    
};
#endif
