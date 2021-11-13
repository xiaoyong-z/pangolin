#ifndef BUILDER_H
#define BUILDER_H
#include <vector>
#include "file.h"
#include "block.h"
#include "util.h"
class Builder {
public:
    Builder(const std::shared_ptr<Options>& opt): opt_(opt) {}

    RC insert(const strEntry& entry) {
        if (cur_block_ == nullptr) {
            cur_block_ = std::make_shared<Block>();
        }
        if (cur_block_->checkFinish(entry, opt_->block_size_)) {
            cur_block_->Finish();
            blocks_.push_back(cur_block_);
            cur_block_ = std::make_shared<Block>();
        }

        cur_block_->insert(entry);

        return RC::SUCCESS;
    }


private:
    std::shared_ptr<Options> opt_;
    std::shared_ptr<Block> cur_block_;
    std::vector<std::shared_ptr<Block>> blocks_;
    int offset;
};
#endif
