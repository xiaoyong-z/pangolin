#ifndef LSM_H
#define LSM_H
#include <string>
#include "memtable.h"
#include "levels.h"
#include "file.h"
#include "util.h"

class LSM {

public:
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
	std::shared_ptr<Options> options;
};
#endif