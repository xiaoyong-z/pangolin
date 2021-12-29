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
	static LSM* NewLSM(std::shared_ptr<Options> options) {
		std::shared_ptr<MemTable> memtable;
		std::vector<std::shared_ptr<MemTable>> immutables;
		LSM* lsm = new LSM(options);

		lsm->level_mangaer_.reset(NewLevelManager(options));

		RC result = RecoveryWAL(options, memtable, immutables, lsm->level_mangaer_.get());
		if (result != RC::SUCCESS) {
			
		}
		return lsm;
	}

	static RC RecoveryWAL(const std::shared_ptr<Options>& options, 
        std::shared_ptr<MemTable>& memtable, std::vector<std::shared_ptr<MemTable>>, LevelManager* level_manager) {

        bool has_wal_file = false;
        std::vector<std::string> wal_file_names;
        for (const auto & entry : std::filesystem::directory_iterator(options->work_dir_)) {
            has_wal_file = true;
            std::cout << entry.path() << std::endl;
        }

		MemTable* memtable_ptr = NewMemTable(options, level_manager);
        // memtable = std::make_shared<MemTable>(options);        
        return RC::SUCCESS;
    }

	static MemTable* NewMemTable(const std::shared_ptr<Options>& options, LevelManager* level_manager) {
        uint32_t fid = level_manager->cur_file_id_.fetch_add(1);

		std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>(fid, options->work_dir_, O_CREAT | O_RDWR, options->ssTable_max_sz_);
		file_opt->file_name_ = Util::filePathJoin(options->work_dir_, fid, "wal");
		

        
		// std::string file_name_ = opt->work_dir_ + "00000" + std::to_string(0) + ".mem";
     
        
        // wal_file_(std::move(wal_file)), skipList_(std::move(skiplist)){};
	}

	static LevelManager* NewLevelManager(const std::shared_ptr<Options>& options){
        LevelManager* level_manger = new LevelManager(options);
        // lm.opt_ = opt_
        // // 读取manifest文件构建管理器
        // if err := lm.loadManifest(); err != nil {
        //     panic(err)
        // }
        // lm.build()
        // return lm
        return level_manger;
    }

	RC Close() {
		RC result;
		// Todo 
		// if (memtable_ != nullptr) {
		// 	result = memtable_->close();
		// }
		return result;
	}
	
	RC Get() {
		return RC::SUCCESS;
	}


private:
	std::shared_ptr<MemTable> memtable_;
	std::vector<std::shared_ptr<MemTable>> immutables_;

	std::unique_ptr<LevelManager> level_mangaer_;
	std::shared_ptr<Options> options_;
};
#endif