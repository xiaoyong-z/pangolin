#include "compaction.h"
Compaction::Compaction(const std::shared_ptr<Options>& options, std::shared_ptr<CompactionState>& state, 
    std::shared_ptr<LevelsManager>& level_manager, int level_id): 
    level_manager_(level_manager), state_(state), this_level_(level_manager->getLevelHandler(level_id)), 
    next_level_(level_manager->getLevelHandler(level_id + 1)), opt_(options), level_id_(level_id), 
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
        if (this_level_->getTableNum() > CompactionConfig::level0CompactionThreadshold) {
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
    std::vector<std::shared_ptr<Table>> two_level_tables(plan_.this_tables_);
    std::copy(plan_.next_tables_.begin(), plan_.next_tables_.end(), std::back_inserter(two_level_tables));
    if (two_level_tables.size() == 0) {
        return;
    }
    std::unique_ptr<TableMergeIterator> iterator = std::make_unique<TableMergeIterator>(two_level_tables);
    std::shared_ptr<Builder> builder;
    std::vector<std::shared_ptr<Builder>> builders;
    std::vector<std::shared_ptr<Table>> new_tables;
    for (; iterator->Valid(); iterator->Next()) {
        if (builder.get() == nullptr) {
            builder = std::make_shared<Builder>(opt_);
            builders.push_back(builder);
        }
        Entry entry;
        iterator->getEntry(entry);
        builder->insert(entry);
        if (builder->checkFinish()) {
            std::shared_ptr<Table> table = level_manager_->newTable();
            table->flush(builder, false);
            new_tables.emplace_back(table);
            builder = nullptr;
        }
    }

    for (size_t i = 0; i < new_tables.size(); i++) {
        new_tables[i]->sync();
        new_tables[i]->open();
    }

    pb::ManifestChangeSet set = ManifestFile::buildChangeSet(plan_, new_tables);

    std::shared_ptr<ManifestFile>& manifest_file = level_manager_->getManifestFile();
    pb::ManifestChangeSet change_set = manifest_file->buildChangeSet(plan_, new_tables);
    RC result = manifest_file->applyChangeSet(change_set);
    assert(result == RC::SUCCESS);

    this_level_->deleteTables(plan_.this_tables_);
    next_level_->replaceTables(plan_.next_tables_, new_tables);
    
    return;
}