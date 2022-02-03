#ifndef COMPACTION_H
#define COMPACTION_H
#include "levelsManager.h"
#include "thread.h"
#include "compactionPlan.h"
#include "compactionState.h"
#include "keyrange.h"
#include "table.h"
#include "manifest.h"
class Table;
class Compaction : public Runnable {
public:
    // id 1 thread handles level 1 compaction, id 2 thread handles level 2 compaction
    Compaction(const std::shared_ptr<Options>& options, std::shared_ptr<CompactionState>& state, 
        std::shared_ptr<LevelsManager>& level_manager, int level_id);

    ~Compaction() override;
    void run() override;
    void init();
    bool needCompaction();
    bool doCompaction();
    bool generateCompactionPlan();
    bool fillTablesL0ToL1();
    bool fillTablesLnToLnp1();
    void performCompaction();

private:
    std::shared_ptr<LevelsManager> level_manager_;
    std::shared_ptr<CompactionState> state_; 
    std::shared_ptr<LevelHandler> this_level_;
    std::shared_ptr<LevelHandler> next_level_;
    std::shared_ptr<Options> opt_;
    int level_id_;
    uint64_t cur_level_max_size_;
    CompactionPlan plan_;
};
#endif