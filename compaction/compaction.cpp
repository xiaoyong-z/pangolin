#include "compaction.h"
Compaction::Compaction(const std::shared_ptr<Options>& options, std::shared_ptr<CompactionState>& state, 
    std::shared_ptr<LevelsManager>& level_manager, int level_id): 
    level_manager_(level_manager), state_(state), this_level_(level_manager->getLevelHandler(level_id)), 
    next_level_(level_manager->getLevelHandler(level_id + 1)), level_id_(level_id), 
    cur_level_max_size_(options->getSSTableSize() * std::pow(options->getLevelSizeMultiplier(), level_id - 1)) {
        plan_.this_level_num_ = level_id;
        plan_.next_level_num_ = level_id + 1;
        plan_.this_level_handler_ = this_level_;
        plan_.next_level_handler_ = next_level_;
    }

Compaction::~Compaction() {

}

void Compaction::run() {
    init();
    while (true) {
        // std::cout << "compactor: " << level_id_ << " working..." <<  std::endl;
        // if (needCompaction()) {
        //     doCompaction();
        // }
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

bool Compaction::doCompaction() {
    if (generateCompactionPlan() == false) {
        return false;
    }

    performCompaction();

    state_->remove(plan_);
    plan_.clear();
    return true;
}

bool Compaction::generateCompactionPlan() {
    if (level_id_ == 0) {
        return fillTablesL0ToL1();
    } else {
        return fillTablesLnToLnp1();
    }
}

bool Compaction::fillTablesL0ToL1() {
    this_level_->RLock();
    std::vector<std::shared_ptr<Table>>& this_level_tables = this_level_->getTables();
    std::vector<std::shared_ptr<Table>> this_level_output;
    KeyRange kr;
    for (size_t i = 0; i < this_level_tables.size(); i++) {
        KeyRange tkr(this_level_tables[i]);
        if (kr.overlapWith(tkr)) {
            this_level_output.push_back(this_level_tables[i]);
            this_level_tables[i]->increaseRef();
            kr.extend(tkr);
        } else {
            break;
        }
    }
    this_level_->UnRLock();


    plan_.this_range_ = std::move(kr);
    plan_.this_tables_ = std::move(this_level_output);

    next_level_->RLock();
    std::vector<std::shared_ptr<Table>>& next_level_tables = next_level_->getTables();
    std::pair<int, int> pair = plan_.this_range_.overlappingTables(next_level_tables);
    std::vector<std::shared_ptr<Table>> next_level_output;
    for (int i = pair.first; i <= pair.second; i++) {
        next_level_output.push_back(next_level_tables[i]);
    }
    next_level_->UnRLock();
    if (next_level_output.size() == 0) {
        plan_.next_range_ = plan_.this_range_;
    } else {
        plan_.next_range_.setKeyRange(next_level_output);
    }
    plan_.next_tables_ = std::move(next_level_output);
    return state_->compareAndAdd(plan_);
}

bool Compaction::fillTablesLnToLnp1() {
    this_level_->RLock();
    next_level_->RLock();
    
    std::vector<std::shared_ptr<Table>>& this_level_tables = this_level_->getTables();
    std::vector<std::shared_ptr<Table>> this_level_sort_tables;
    this_level_sort_tables.assign(this_level_tables.begin(), this_level_tables.end());
    sort(this_level_sort_tables.begin(), this_level_sort_tables.end(), 
        [](const std::shared_ptr<Table>& a, const std::shared_ptr<Table>& b) -> bool{ 
            return a->getMaxVersion() > b->getMaxVersion(); 
        });

    for (size_t i = 0; i < this_level_tables.size(); i++) {
        std::shared_ptr<Table>& table = this_level_tables[i];
        KeyRange tkr(table);
        if (state_->overlapsWith(plan_.this_level_num_, tkr)) {
            continue;
        }

        std::vector<std::shared_ptr<Table>>& next_level_tables = next_level_->getTables();
        std::pair<int, int> pair = plan_.this_range_.overlappingTables(next_level_tables);
        std::vector<std::shared_ptr<Table>> next_level_output;
        for (int i = pair.first; i <= pair.second; i++) {
            next_level_output.push_back(next_level_tables[i]);
        }
        KeyRange nkr(next_level_output);
        if (state_->overlapsWith(plan_.next_level_num_, nkr)) {
            continue;
        }
        plan_.this_tables_ = {table};
        plan_.this_range_ = std::move(tkr);
        plan_.next_tables_ = std::move(next_level_output);
        plan_.next_range_ = std::move(nkr);
        if (state_->compareAndAdd(plan_) == false) {
            continue;
        }
    }

    this_level_->UnRLock();
    next_level_->UnRLock();
    return true;
}

void Compaction::performCompaction() {
    
}