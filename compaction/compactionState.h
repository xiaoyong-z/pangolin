#ifndef COMPACTION_STATE_H
#define COMPACTION_STATE_H
#include "util.h"
#include "table.h"
#include "keyrange.h"
#include "compactionPlan.h"
class CompactionState {
public:
    CompactionState(int max_level_num) {
        for (int i = 0; i < max_level_num; i++) {
            levels_ranges_.emplace_back();
            levels_table_set_.emplace_back();
        }
    }

    bool overlapsWith(int level_num, const KeyRange& another_range, bool hasLock = false) {
        if (hasLock == false) {
            rwLock_.lock_shared();
        }
        bool result = false;
        for (const auto& range: levels_ranges_[level_num]) {
            if (range.overlapWith(another_range)) {
                result = true;
                break;
            }
        }
        if (hasLock == false) {
            rwLock_.unlock_shared();
        }
        return result;
    }

    bool compareAndAdd(const CompactionPlan& plan) {
        WriteLock lock(rwLock_);
        if (overlapsWith(plan.this_level_num_, plan.this_range_, true)) {
            return false;
        }
        if (overlapsWith(plan.next_level_num_, plan.next_range_, true)) {
            return false;
        }
        if (plan.this_range_.isEmpty() == false) {
            levels_ranges_[plan.this_level_num_].push_back(plan.this_range_);
        }
        if (plan.next_range_.isEmpty() == false) {
            levels_ranges_[plan.next_level_num_].push_back(plan.next_range_);
        }
        copy(plan.this_tables_.begin(), plan.this_tables_.end(), inserter(levels_table_set_[plan.this_level_num_], 
            levels_table_set_[plan.this_level_num_].begin()));

        copy(plan.next_tables_.begin(), plan.next_tables_.end(), inserter(levels_table_set_[plan.next_level_num_], 
            levels_table_set_[plan.next_level_num_].begin()));

        return true;
    }

    

    bool remove(const CompactionPlan& plan) {
        WriteLock lock(rwLock_);
        eraseKeyRange(plan.this_level_num_, plan.this_range_);
        eraseKeyRange(plan.next_level_num_, plan.next_range_);

        const std::vector<std::shared_ptr<Table>>& this_tables = plan.this_tables_;
        const std::vector<std::shared_ptr<Table>>& next_tables = plan.next_tables_; 
        
        for (size_t i = 0; i < this_tables.size(); i++) {
            levels_table_set_[plan.this_level_num_].erase(this_tables[i]);
        }

        for (size_t i = 0; i < next_tables.size(); i++) {
            levels_table_set_[plan.next_level_num_].erase(next_tables[i]);
        }
        return true;
    }

private:
    // before calling this function, ensure acquire the write lock
    bool eraseKeyRange(int level_num, const KeyRange& range) {
        std::vector<KeyRange>& cur_level_ranges = levels_ranges_[level_num];
        int level_range_num = cur_level_ranges.size();
        for (int i = 0;  i < level_range_num; i++) {
            if (cur_level_ranges[i].equals(range)) {
                cur_level_ranges.erase(cur_level_ranges.begin() + i);
                return true;
            }
        }
        return false;
    }

    RWLock rwLock_;
    std::vector<std::vector<KeyRange>> levels_ranges_;
    std::vector<std::set<std::shared_ptr<Table>>> levels_table_set_;
};
#endif