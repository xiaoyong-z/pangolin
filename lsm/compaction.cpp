#include "compaction.h"
Compaction::Compaction(const std::shared_ptr<Options>& options_, std::shared_ptr<LevelsManager>& level_manager, int level_id): 
    level_manager_(level_manager), this_level_(level_manager->getLevelHandler(level_id)), next_level_(level_manager->getLevelHandler(level_id + 1)),
    level_id_(level_id), cur_level_max_size_(options_->getSSTableSize() * std::pow(options_->getLevelSizeMultiplier(), level_id - 1)) {}

Compaction::~Compaction() {

}

void Compaction::run() {
    init();
    while (true) {
        std::cout << "compactor: " << level_id_ << " working..." <<  std::endl;
        if (needCompaction()) {
            doCompaction();
        }
        sleep(CompactionConfig::compaction_duration);
    }
}

void Compaction::init() {}

bool Compaction::needCompaction() {
    if (level_id_ == 0) {
        if (CompactionConfig::level0CompactionThreadshold) {
            return true;
        }
    } else {
        if (this_level_->getLevelSize() > cur_level_max_size_) {
            return true;
        }
    }
    return false;
}

void Compaction::doCompaction() {
    generateCompactionPlan();
}

void Compaction::generateCompactionPlan() {
    if (level_id_ == 0) {
        fillTablesL0ToL1();
    } else {
        fillTablesLnToLnp1();
    }
}

void Compaction::fillTablesL0ToL1() {
    this_level_->Lock();
    std::vector<std::shared_ptr<Table>>& tables = this_level_->getTables();
    std::vector<std::shared_ptr<Table>> this_level_tables;
    KeyRange kr;
    for (size_t i = 0; i < tables.size(); i++) {
        KeyRange tkr(tables[i]);
        if (kr.overlapWith(tkr)) {
            this_level_tables.push_back(tables[i]);
            tables[i]->increaseRef();
            kr.extend(tkr);
        } else {
            break;
        }
    }
    this_level_->Unlock();
}

void Compaction::fillTablesLnToLnp1() {
    this_level_->Lock();
    this_level_->Unlock();
}