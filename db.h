#ifndef DB_H
#define DB_H

#include "lsm.h"
#include "skipLists.h"
#include "levelsManager.h"
#include "memtable.h"
#include "wal.h"
#include "file.h"
#include "util.h"

class DB {
public:
	DB(std::shared_ptr<Options> options);
	~DB();
    void del(const std::string& key);
	void set(const std::string& key, const std::string& value);
	std::string get(const std::string& key);
private:
    std::shared_ptr<Options> opt_;
	std::shared_ptr<LSM> lsm_;
};

#endif