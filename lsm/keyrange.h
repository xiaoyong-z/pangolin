#ifndef KEYRANGE_H
#define KEYRANGE_H
#include "util.h"
#include "table.h"

class KeyRange {
public:
    KeyRange(): empty_(true) {}

    KeyRange(const std::shared_ptr<Table>& table): 
        min_key_(table->getMinKey()), max_key_(table->getMaxKey()), empty_(false) {}
    
    KeyRange(const std::vector<std::shared_ptr<Table>>& tables): empty_(false) {
        if (tables.size() == 0) {
            return;
        }
        min_key_ = tables[0]->getMinKey();
        max_key_ = tables[0]->getMaxKey();
        for (size_t i = 1; i < tables.size(); i++) {
            if (Util::compareKey(tables[i]->getMinKey(), min_key_) < 0) {
                min_key_ = tables[i]->getMinKey();
            }
            if (Util::compareKey(tables[i]->getMaxKey(), max_key_) > 0) {
                max_key_ = tables[i]->getMaxKey();
            }
        }
    }

    bool overlapWith(const KeyRange& range) {
        if (empty_) {
            return true;
        }
        if (Util::compareKey(min_key_, range.min_key_) > 0) {
            return false;
        }

        if (Util::compareKey(max_key_, range.max_key_) < 0) {
            return false;
        }
        return true;
    }

    void extend(const KeyRange& range) {
        if (isEmpty()) {
            min_key_ = range.min_key_;
            max_key_ = range.max_key_;
            return;
        }

        if (Util::compareKey(min_key_, range.min_key_) > 0) {
            min_key_ = range.min_key_;
        }

        if (Util::compareKey(max_key_, range.max_key_) < 0) {
            max_key_ = range.max_key_;
        }
        return;
    }

    inline const bool isEmpty() const{
        return empty_;
    }

    bool equals(const KeyRange& range) {
        if (Util::compareBytes(min_key_, range.min_key_) == 0 && 
            Util::compareBytes(max_key_, range.max_key_) == 0) {
            return true;
        }
        return false;
    }

private:
    std::string min_key_;
    std::string max_key_;
    bool empty_;
};
#endif