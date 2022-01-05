#ifndef LEVELS_H
#define LEVELS_H
#include "file.h"
#include "memtable.h"
#include "builder.h"
#include "table.h"
#include "cache.h"
#include "manifest.h"
class LevelManager;
class LevelHandler {
public:
    RC level0Get(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {
        for (size_t i = 0; i < tables_.size(); i++) {
            if (tables_[i]->get(key, entry, opt) == RC::SUCCESS) {
                return RC::SUCCESS;
            }
        }
        return RC::LEVELS_KEY_NOT_FOUND_IN_CUR_LEVEL;
    }

    RC levelNGet(const Slice& key, Entry& entry, const std::shared_ptr<Options>& opt) {

        return RC::SUCCESS;
    }
private:
    friend class LevelManager;
    int level_num_;
    std::vector<std::shared_ptr<Table>> tables_;
};

class LevelManager: public std::enable_shared_from_this<LevelManager> {
    friend class Table;
    friend class LSM;
public:
    LevelManager(const std::shared_ptr<Options>& options, ManifestFile* manifest_file): 
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
        for (const auto& file_id : sstable_file_id) {
            Manifest::TableManifest& table_manifest = tables_manifest[file_id];
            if (file_id > max_fid) {
                max_fid = file_id;
            }
            uint32_t level = table_manifest.level_;
            uint32_t crc = table_manifest.crc_;
            Table* table_raw = Table::NewTable(opt_->work_dir_, file_id, opt_->ssTable_max_sz_);
            assert(table_raw != nullptr);
            std::shared_ptr<Table> table(table_raw);
            if (crc != table->getCRC()) {
                assert(false);
            }
            table->open();
            for (size_t i = levels_.size(); i <= level; i++) {
                levels_.emplace_back();
            }
            levels_[level].tables_.push_back(table);
        }

        cur_file_id_.fetch_add(max_fid);

        if (levels_.size() == 0) {
            levels_.emplace_back();
        }
        // std::vector<
    }

    RC get(const Slice& key, Entry& entry) {
        return levels_[0].level0Get(key, entry, opt_);
    }

    RC flush(const std::shared_ptr<MemTable>& memtable) {
        uint32_t file_id = cur_file_id_.fetch_add(1);
        Table* table_raw = Table::NewTable(opt_->work_dir_, file_id, opt_->ssTable_max_sz_);
        if (table_raw == nullptr) {
            return RC::LEVELS_FILE_NOT_OPEN;
        }
        std::shared_ptr<Table> table(table_raw);
        std::shared_ptr<Builder> builder = std::make_shared<Builder>(opt_);
        std::unique_ptr<SkipListIterator> iterator(memtable->skipList_->newIterator());
        for (; iterator->hasNext() ; iterator->next()) {
            builder->insert(iterator->get());
        }
        table->flush(builder);
        table->open();
        levels_[0].tables_.push_back(table);
        manifest_file_->addTableMeta(0, table);

        return RC::SUCCESS;
    }

private:
    std::atomic<uint32_t> cur_file_id_;
    std::shared_ptr<Options> opt_;
    std::vector<LevelHandler> levels_;
    std::unique_ptr<ManifestFile> manifest_file_;
    Cache cache;
};


#endif