#ifndef COMPACTION_H
#define COMPACTION_H
#include "levelsManager.h"
#include "thread.h"
class Compaction : public Runnable {
public:
    // compaction id 1 handles level 1 compaction, id 2 handles level 2 compaction
    Compaction(std::shared_ptr<LevelsManager>& level_manager, int compaction_id): 
        level_manager_(level_manager), compaction_id_(compaction_id) {}
    
    ~Compaction() override {}

    void run() override {
        while (true) {
            std::cout << "compactor: " << compaction_id_ << " working..." <<  std::endl;
            

            sleep(CompactionConfig::compaction_duration);
        }
    }

private:

    std::shared_ptr<LevelsManager> level_manager_;
    int compaction_id_;
};
#endif