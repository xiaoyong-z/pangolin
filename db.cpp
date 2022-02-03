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
    Slice skey(key);
    Slice svalue("");
    Entry entry(skey, svalue);
    RC rc = lsm_->set(&entry);
    assert(rc == RC::SUCCESS);
}

void DB::set(const std::string& key, const std::string& value) {
    Slice skey(key);
    Slice svalue(value);
    Entry entry(skey, svalue);
    RC rc = lsm_->set(&entry);
    assert(rc == RC::SUCCESS);
}

std::string DB::get(const std::string& key) {
    Slice skey(key);
    Entry result;
    RC rc = lsm_->get(skey, result);
    assert(rc == RC::SUCCESS);
    return result.value_.ToString();
}

// Used for debug only
void DB::scan() {
    lsm_->scan();
}