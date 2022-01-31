#include "levelsManager.h"

LevelsManager::LevelsManager(const std::shared_ptr<Options>& options, ManifestFile* manifest_file): 
    cur_file_id_(0), opt_(options), manifest_file_(manifest_file) {
    std::set<uint32_t> sstable_file_id;
    for (const auto & entry : std::filesystem::directory_iterator(options->work_dir_)) {
        const std::string& path = entry.path().string();
        int suffix_position = path.find_last_of('.');
        int last_slash_postion = path.find_last_of("/");
        if (path.substr(suffix_position + 1) == SSTableConfig::filePostfix) {
            sstable_file_id.insert(std::stoi(path.substr(last_slash_postion + 1, suffix_position - last_slash_postion - 1)));
        }
    }
    RC result = manifest_file_->revertToManifest(sstable_file_id);
    assert(result == RC::SUCCESS);
    uint32_t max_fid = 0;
    std::unordered_map<uint32_t, Manifest::TableManifest>& tables_manifest = manifest_file_->getTables();

    int max_level_num = options->getMaxLevelNum();
    for (int i = 0; i <= max_level_num; i++) {
        levels_.emplace_back(new LevelHandler(i));
    }

    for (const auto& file_id : sstable_file_id) {
        Manifest::TableManifest& table_manifest = tables_manifest[file_id];
        if (file_id > max_fid) {
            max_fid = file_id;
        }
        uint32_t level = table_manifest.level_;
        uint32_t crc = table_manifest.crc_;
        Table* table_raw = Table::NewTable(opt_->work_dir_, file_id, opt_->SSTable_max_sz);
        assert(table_raw != nullptr);
        std::shared_ptr<Table> table(table_raw);
        table->open();
        if (crc != table->getCRC()) {
            assert(false);
        }
        levels_[level]->flush(table);
    }

    cur_file_id_.fetch_add(max_fid + 1);
}

RC LevelsManager::get(const Slice& key, Entry& entry) {
    return levels_[0]->level0Get(key, entry, opt_);
}

RC LevelsManager::flush(std::shared_ptr<MemTable>& memtable) {
    uint32_t file_id = cur_file_id_.fetch_add(1);
    Table* table_raw = Table::NewTable(opt_->work_dir_, file_id, opt_->SSTable_max_sz);
    if (table_raw == nullptr) {
        return RC::LEVELS_FILE_NOT_OPEN;
    }
    std::shared_ptr<Table> table(table_raw);
    std::shared_ptr<Builder> builder = std::make_shared<Builder>(opt_);
    std::shared_ptr<SkipListIterator> iterator = SkipListIterator::NewIterator(memtable->getSkipList());
    for (; iterator->Valid() ; iterator->Next()) {
        Entry entry;
        iterator->getEntry(entry);
        builder->insert(entry);
    }
    table->flush(builder);
    memtable->close();
    table->open();
    
    levels_[0]->flush(table);
    manifest_file_->addTableMeta(0, table);

    return RC::SUCCESS;
}


uint64_t LevelsManager::getLevelSize(int level_num) {
    assert(level_num < opt_->getMaxLevelNum());
    return levels_[level_num]->getTableNum();
 }

std::vector<std::shared_ptr<Table>>& LevelsManager::getTables(int level_num) {
    assert(level_num < opt_->getMaxLevelNum());
    return levels_[level_num]->getTables();
}

std::shared_ptr<LevelHandler>& LevelsManager::getLevelHandler(int level_num) {
    assert(level_num <= opt_->getMaxLevelNum());
    return levels_[level_num];

}
