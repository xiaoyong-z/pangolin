#ifndef KEYRANGE_H
#define KEYRANGE_H
#include "util.h"
#include "table.h"

class KeyRange {
public:
    KeyRange() noexcept: empty_(true) {}

    KeyRange(const std::shared_ptr<Table>& table) noexcept: 
        min_key_(table->getMinKey()), max_key_(table->getMaxKey()), empty_(false) {}
    
    KeyRange(const std::vector<std::shared_ptr<Table>>& tables) noexcept: empty_(false) {
        setKeyRange(tables);
    }

    KeyRange(KeyRange&& range) noexcept:
        min_key_(std::move(range.min_key_)), 
        max_key_(std::move(range.max_key_)), empty_(range.empty_) {}
    
    KeyRange(const KeyRange& range):
        min_key_(range.min_key_), max_key_(range.max_key_), empty_(range.empty_) {}

    KeyRange& operator=(KeyRange&& range) {
        min_key_ = std::move(range.min_key_);
        max_key_ = std::move(range.max_key_);
        empty_ = range.empty_;
        return *this;
    }

    KeyRange& operator=(const KeyRange& range) {
        min_key_ = range.min_key_;
        max_key_ = range.max_key_;
        empty_ = range.empty_;
        return *this;
    }

    void setKeyRange(const std::vector<std::shared_ptr<Table>>& tables) {
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

    bool overlapWith(const KeyRange& range) const {
        if (empty_) {
            return true;
        }
        if (Util::compareKey(min_key_, range.max_key_) > 0) {
            return false;
        }

        if (Util::compareKey(max_key_, range.min_key_) < 0) {
            return false;
        }
        return true;
    }

    static bool compare(const std::shared_ptr<Table>& a, const std::shared_ptr<Table>& b){
        return Util::compareKey(a->getMaxKey(), b->getMaxKey()) < 0;
    }
    
    int lower_bound(const std::vector<std::shared_ptr<Table>>& tables) {
        int left = 0, right = tables.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int cmp = Util::compareKey(min_key_, tables[mid]->getMaxKey());
            if (cmp == 0)
                return mid; 
            else if (cmp < 0)
                left = mid + 1;
            else
                right = mid - 1;
        }
        return left;
    }

    int upper_bound(const std::vector<std::shared_ptr<Table>>& tables) {
        int left = 0, right = tables.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int cmp = Util::compareKey(max_key_, tables[mid]->getMaxKey());
            if (cmp == 0)
                return mid; 
            else if (cmp < 0)
                left = mid + 1;
            else
                right = mid - 1;
        }
        return left;
    }

    // tables here is sorted
    std::pair<int, int> overlappingTables(std::vector<std::shared_ptr<Table>>& tables) {
        int left = lower_bound(tables);
        int right = upper_bound(tables);
        return {left, right};
    }

    void extend(const KeyRange& range) {
        if (isEmpty()) {
            min_key_ = range.min_key_;
            max_key_ = range.max_key_;
            setNotEmpty();
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

    inline void setNotEmpty() {
        empty_ = false;
    }

    bool equals(const KeyRange& range) {
        if (Util::compareBytes(min_key_, range.min_key_) == 0 && 
            Util::compareBytes(max_key_, range.max_key_) == 0) {
            return true;
        }
        return false;
    }

    void clear() {
        min_key_ = "";
        max_key_ = "";
        empty_ = true;
    }

private:
    std::string min_key_;
    std::string max_key_;
    bool empty_;
};
#endif