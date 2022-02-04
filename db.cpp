#include "db.h"

DB::DB(std::shared_ptr<Options> options): opt_(options) {
    LSM* lsm = LSM::newLSM(opt_);
    assert (lsm != nullptr);
    lsm_.reset(lsm);
}

DB::~DB() {
    RC rc = lsm_->flush();
    assert(rc == RC::SUCCESS);
}

void DB::del(const std::string& key) {
    RC rc = lsm_->set(key, "", true);
    assert(rc == RC::SUCCESS);
}

void DB::set(const std::string& key, const std::string& value) {
    RC rc = lsm_->set(key, value);
    assert(rc == RC::SUCCESS);
}

std::string DB::get(const std::string& key) {
    std::string value;
    lsm_->get(key, value);
    // assert(rc == RC::SUCCESS);
    return value;
}

// Used for debug only
void DB::scan() {
    lsm_->scan();
}