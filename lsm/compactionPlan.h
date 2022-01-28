#ifndef COMPACTION_PLAN_H
#define COMPACTION_PLAN_H
#include "keyrange.h"
#include "table.h"
#include "levelHandler.h"
struct compactionPlan {
    compactionPlan(int compactor_id, const KeyRange& this_range, const KeyRange& next_range, 
        const std::vector<std::shared_ptr<Table>>& this_tables, const std::vector<std::shared_ptr<Table>>& next_tables, 
        const std::shared_ptr<LevelHandler>& this_level_handler, const std::shared_ptr<LevelHandler>& next_level_handler,
        int this_level_num, int next_level_num): compactor_id_(compactor_id), this_range_(this_range), next_range_(next_range),
        this_tables_(this_tables), next_tables_(next_tables), this_level_handler_(this_level_handler), next_level_handler_(next_level_handler),
        this_level_num_(this_level_num), next_level_num_(next_level_num) {}

    int compactor_id_;
    KeyRange this_range_;
    KeyRange next_range_;
    std::vector<std::shared_ptr<Table>> this_tables_;
    std::vector<std::shared_ptr<Table>> next_tables_;
    std::shared_ptr<LevelHandler> this_level_handler_;
    std::shared_ptr<LevelHandler> next_level_handler_;
    int this_level_num_;
    int next_level_num_;
};
#endif