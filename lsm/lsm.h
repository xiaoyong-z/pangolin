#ifndef LSM_H
#define LSM_H
#include <string>
#include "memtable.h"
#include "compaction.h"
#include "levelsManager.h"
#include "file.h"
#include "util.h"
#include "compactionState.h"

class LSM {
	LSM(std::shared_ptr<Options> options):last_version_(0), options_(options) {
		compaction_state_ = std::make_shared<CompactionState>(options_->getCompactionThreadNum());
	}
public:
	~LSM() {
		stopCompaction();
	}

	static LSM* newLSM(std::shared_ptr<Options> options) {
		std::shared_ptr<MemTable> memtable;
		std::vector<std::shared_ptr<MemTable>> immutables;
		LSM* lsm = new LSM(options);

		ManifestFile* manifest_file = newManifest(options);
		lsm->manifest_file_.reset(manifest_file);
		lsm->level_manager_.reset(newLevelManager(options, lsm->manifest_file_));

		RC result = recoveryWAL(options, lsm->memtable_, lsm->immutables_, lsm->level_manager_);
		if (result != RC::SUCCESS) {
			return nullptr;
		}
		result = lsm->startCompaction();
		if (result != RC::SUCCESS) {
			return nullptr;
		}
		return lsm;
	}

	static RC recoveryWAL(const std::shared_ptr<Options>& options, std::shared_ptr<MemTable>& memtable, 
		std::vector<std::shared_ptr<MemTable>>& immutables, std::shared_ptr<LevelsManager>& level_manager) {

        std::map<int, std::string> wal_map;
        for (const auto & entry : std::filesystem::directory_iterator(options->work_dir_)) {
			const std::string& path = entry.path().string();
			int suffix_position = path.find_last_of('.');
			int last_slash_postion = path.find_last_of("/");
			if (path.substr(suffix_position + 1) == WALConfig::filePostfix) {
				wal_map.emplace(std::stoi(path.substr(last_slash_postion + 1, suffix_position - last_slash_postion - 1)), path);
				// wal_names.push_back(path);
			}
        }

		for (const auto & iterator: wal_map) {
			std::shared_ptr<MemTable> memtable = openMemTable(options, iterator.first);
			if (memtable->getEntryCount() == 0) {
				continue;
			}
			immutables.push_back(memtable);
			// std::cout << ite.second << std::endl;
		}

		memtable = newMemTable(options, level_manager);   
        return RC::SUCCESS;
    }

	static std::shared_ptr<MemTable> newMemTable(const std::shared_ptr<Options>& options, std::shared_ptr<LevelsManager>& level_manager) {
        uint32_t fid = level_manager->cur_file_id_.fetch_add(1);

		std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>(fid, options->work_dir_, O_CREAT | O_RDWR, options->SSTable_max_sz);
		file_opt->file_name_ = Util::filePathJoin(options->work_dir_, fid, WALConfig::filePostfix);
		WALFile* wal = WALFile::newWALFile(file_opt);
		assert(wal != nullptr);
		SkipList* skiplist = new SkipList();
		return std::make_shared<MemTable>(wal, skiplist); 
	}

	static std::shared_ptr<MemTable> openMemTable(const std::shared_ptr<Options>& options, const uint32_t fid) {
		std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>(fid, options->work_dir_, O_RDONLY, options->SSTable_max_sz);
		file_opt->file_name_ = Util::filePathJoin(options->work_dir_, fid, WALConfig::filePostfix);
		WALFile* wal = WALFile::newWALFile(file_opt);
		assert(wal != nullptr);
		SkipList* skiplist = new SkipList();
		std::shared_ptr<MemTable> memTable = std::make_shared<MemTable>(wal, skiplist);
		memTable->updateList(options);
		return memTable;
	}

	static ManifestFile* newManifest(const std::shared_ptr<Options>& options) {
		ManifestFile* manifest_file = ManifestFile::openManifestFile(options);
		assert (manifest_file != nullptr);
        return manifest_file;
    }

	static LevelsManager* newLevelManager(const std::shared_ptr<Options>& options, std::shared_ptr<ManifestFile> manifest_file) {
        LevelsManager* level_manger = new LevelsManager(options, manifest_file);
        return level_manger;
    }

	RC set(const std::string& key, const std::string& value, bool del = false) {

		Slice skey(key);
    	Slice svalue(value);
    	Entry entry(skey, svalue);

		if (memtable_->wal_->size() + entry.estimateWalEntrySize() > options_->mem_table_size_) {
			immutables_.push_back(memtable_);
			memtable_ = newMemTable(options_, level_manager_);
		}

		if (del) {
			entry.setVersionNumAndType(last_version_.fetch_add(1), kTypeDeletion);
		} else {
			entry.setVersionNumAndType(last_version_.fetch_add(1), kTypeValue);
		}

		RC result = memtable_->set(&entry);
		if (result != RC::SUCCESS) {
			return result;
		}

		for (size_t i = 0; i < immutables_.size(); i++) {
			level_manager_->flush(immutables_[i]);
		}
		immutables_.clear();
		return RC::SUCCESS;
    }


	RC close() {
		RC result;
		// Todo 
		// if (memtable_ != nullptr) {
		// 	result = memtable_->close();
		// }
		return result;
	}
	
	RC get(const std::string& key, std::string& value) {
		std::string internalKey = key;
		uint32_t last_version = last_version_.load();
		Util::encodeVersionNum(&internalKey, last_version);
		Slice skey(internalKey);

		Entry entry;
    	// Slice svalue(value);
    	// Entry entry(skey, svalue);
		// entry.setVersionNumAndType(last_version_.load());
		RC rc = memtable_->get(skey, entry);
		if (rc == RC::SUCCESS) {
			value = entry.getValue().ToString();
			return rc;
		}

		for (size_t i = 0; i < immutables_.size(); i++) {
			rc = immutables_[i]->get(skey, entry);
			if (rc == RC::SUCCESS) {
				value = entry.getValue().ToString();
				return rc;
			}
		}

		rc = level_manager_->get(skey, entry);
		if (rc == RC::SUCCESS) {
			value = entry.getValue().ToString();
		}
		return rc;
	}

	void scan() {
		std::cout << "==================" << std::endl;
		std::cout << "memtable : " << std::endl;
		memtable_->scan();
		std::cout << "==================" << std::endl;
		std::cout << "immutable memtables : " << std::endl;
		for (size_t i = 0; i < immutables_.size(); i++) {
			std::cout << "immutable table i : " << i << ". " << std::endl;
			immutables_[i]->scan();
		}
		std::cout << "==================" << std::endl;
		std::cout << "levels : " << std::endl;
		level_manager_->scan();
	}

	RC flush() {
		std::cout << "called flush " << std::endl;
		// scan();

		if (memtable_->getEntryCount() > 0) { 
			RC result = level_manager_->flush(memtable_);
			if (result != RC::SUCCESS) {
				return result;
			}
			// memtable_ = newMemTable(options_, level_manager_);
		}
		return RC::SUCCESS;
	}

	RC stopCompaction() {
		for (size_t i = 0; i < threads_.size(); i++) {
			threads_[i]->stop();
		}
		return RC::SUCCESS;
	}

	RC startCompaction() {
		int compaction_thread_num = options_->getCompactionThreadNum();
		for (int i = 0; i < compaction_thread_num; i++) {
			threads_.emplace_back(std::make_unique<Thread>());
			Compaction* compaction = new Compaction(options_, compaction_state_, getLevelManager(), i);
			threads_[i]->start(compaction);
		}
		return RC::SUCCESS;
	}

	inline std::shared_ptr<LevelsManager>& getLevelManager() {
		return level_manager_;
	}

	void manualCompaction() {
		int compaction_thread_num = options_->getCompactionThreadNum();
		for (int i = 0; i < compaction_thread_num; i++) {
			std::shared_ptr<Compaction> compaction = std::make_shared<Compaction>(options_, compaction_state_, getLevelManager(), i);
			compaction->doOnce();
		}
	}


private:
	std::atomic<uint32_t> last_version_; 

	std::shared_ptr<MemTable> memtable_;
	std::vector<std::shared_ptr<MemTable>> immutables_;
	std::shared_ptr<LevelsManager> level_manager_;
	std::shared_ptr<Options> options_;
	std::shared_ptr<ManifestFile> manifest_file_;
	std::vector<std::unique_ptr<Thread>> threads_;
	std::shared_ptr<CompactionState> compaction_state_;
};
#endif