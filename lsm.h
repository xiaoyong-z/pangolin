#ifndef LSM_H
#define LSM_H
#include <string>
#include "memtable.h"
#include "levels.h"
#include "file.h"
#include "util.h"

class LSM {
	LSM(std::shared_ptr<Options> options):options_(options) {

	}
public:
	static LSM* newLSM(std::shared_ptr<Options> options) {
		std::shared_ptr<MemTable> memtable;
		std::vector<std::shared_ptr<MemTable>> immutables;
		LSM* lsm = new LSM(options);

		lsm->level_manager_.reset(newLevelManager(options));

		RC result = recoveryWAL(options, lsm->memtable_, lsm->immutables_, lsm->level_manager_.get());
		if (result != RC::SUCCESS) {
			
		}
		return lsm;
	}

	static RC recoveryWAL(const std::shared_ptr<Options>& options, std::shared_ptr<MemTable>& memtable, 
		std::vector<std::shared_ptr<MemTable>>& immutables, LevelManager* level_manager) {

        std::map<int, std::string> wal_file_map;
        for (const auto & entry : std::filesystem::directory_iterator(options->work_dir_)) {
			const std::string& path = entry.path().string();
			int suffix_position = path.find_last_of('.');
			int last_slash_postion = path.find_last_of("/");
			if (path.substr(suffix_position + 1) == "wal") {
				wal_file_map.emplace(std::stoi(path.substr(last_slash_postion + 1, suffix_position - last_slash_postion - 1)), path);
				// wal_file_names.push_back(path);
			}
        }

		for (const auto & iterator: wal_file_map) {
			immutables.push_back(openMemTable(options, iterator.first));
			// std::cout << ite.second << std::endl;
		}

		memtable = newMemTable(options, level_manager);   
        return RC::SUCCESS;
    }

	static std::shared_ptr<MemTable> newMemTable(const std::shared_ptr<Options>& options, LevelManager* level_manager) {
        uint32_t fid = level_manager->cur_file_id_.fetch_add(1);
		// uint32_t fid = 1;

		std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>(fid, options->work_dir_, O_CREAT | O_RDWR, options->ssTable_max_sz_);
		file_opt->file_name_ = Util::filePathJoin(options->work_dir_, fid, "wal");
		WALFile* wal_file = WALFile::newWALFile(file_opt);
		assert(wal_file != nullptr);
		SkipList* skiplist = new SkipList();
		return std::make_shared<MemTable>(wal_file, skiplist); 
	}

	static std::shared_ptr<MemTable> openMemTable(const std::shared_ptr<Options>& options, const uint32_t fid) {
		std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>(fid, options->work_dir_, O_RDONLY, options->ssTable_max_sz_);
		file_opt->file_name_ = Util::filePathJoin(options->work_dir_, fid, "wal");
		WALFile* wal_file = WALFile::newWALFile(file_opt);
		assert(wal_file != nullptr);
		SkipList* skiplist = new SkipList();
		std::shared_ptr<MemTable> memTable = std::make_shared<MemTable>(wal_file, skiplist);
		memTable->updateList(options);
		return memTable;
	}

	static LevelManager* newLevelManager(const std::shared_ptr<Options>& options) {
		ManifestFile* manifest_file = ManifestFile::openManifestFile(options);
		// ManifestFile* manifest_file_ = new ;
        LevelManager* level_manger = new LevelManager(options, manifest_file);
        // lm.opt_ = opt_
        // // 读取manifest文件构建管理器
        // if err := lm.loadManifest(); err != nil {
        //     panic(err)
        // }
        // lm.build()
        // return lm
        return level_manger;
    }

	RC set(Entry* entry) {
		if (memtable_->wal_file_->size() + entry->estimateWalEntrySize() > options_->mem_table_size_) {
			immutables_.push_back(memtable_);
			memtable_ = newMemTable(options_, level_manager_.get());
		}

		RC result = memtable_->set(entry);
		if (result != RC::SUCCESS) {
			return result;
		}

		for (size_t i = 0; i < immutables_.size(); i++) {
			level_manager_->flush(immutables_[i]);
		}
		immutables_.clear();
    }


	RC close() {
		RC result;
		// Todo 
		// if (memtable_ != nullptr) {
		// 	result = memtable_->close();
		// }
		return result;
	}
	
	RC get(const Slice& key, Entry& entry) {
		RC rc = memtable_->get(key, entry);
		if (rc == RC::SUCCESS) {
			return rc;
		}

		for (size_t i = 0; i < immutables_.size(); i++) {
			rc = immutables_[i]->get(key, entry);
			if (rc == RC::SUCCESS) {
				return rc;
			}
		}

		rc = level_manager_->get(key, entry);
		return rc;
	}

	RC flush() {
		RC result = level_manager_->flush(memtable_);
        if (result != RC::SUCCESS) {
            return result;
        }
		memtable_ = newMemTable(options_, level_manager_.get());
		return RC::SUCCESS;
	}


private:
	std::shared_ptr<MemTable> memtable_;
	std::vector<std::shared_ptr<MemTable>> immutables_;

	std::unique_ptr<LevelManager> level_manager_;
	std::shared_ptr<Options> options_;
};
#endif