#include "manifest.h"

ManifestFile::ManifestFile(std::shared_ptr<Options> opt): 
    opt_(opt), manifest_(new Manifest()){}

void ManifestFile::setFile(int file) {
    file_ = file;
}

ManifestFile::~ManifestFile() {
    std::cout << "manifest freed" << std::endl;
    assert (close(file_) == 0);
}

ManifestFile* ManifestFile::openManifestFile(std::shared_ptr<Options> opt) {
    ManifestFile* manifestFile = new ManifestFile(opt);
    std::string file_name = opt->work_dir_ + ManifestConfig::fileName;
    struct stat statbuf;
    bool flag = stat(file_name.c_str(), &statbuf);
    int file;
    if (flag != 0) {
        RC result = manifestFile->rewriteManifest();
        if (result != RC::SUCCESS) {
            delete manifestFile;
            return nullptr;
        }
        file = open(file_name.c_str(), O_RDWR, 0666);
        if (lseek(file, 0, SEEK_END) == -1) {
            return nullptr;
        }
        assert(file > 0);
        manifestFile->setFile(file);
    } else {
        file = open(file_name.c_str(), O_RDWR, 0666);
        assert(file > 0);
        manifestFile->setFile(file); 
        RC result = manifestFile->replayManifest();
        if (result != RC::SUCCESS) {
            return nullptr;
        }
        if (lseek(file, 0, SEEK_END) == -1) {
            return nullptr;
        }
    }
    return manifestFile;
}

RC ManifestFile::replayManifest() {
    char buf[ManifestConfig::manifestHeadSize];
    size_t count = read(file_, buf, ManifestConfig::manifestHeadSize);
    if (count != ManifestConfig::manifestHeadSize) {
        return RC::MANIFEST_REPLAY_FAIL;
    }
    
    if (memcmp(buf, ManifestConfig::magicNum.data(), ManifestConfig::magicNum.size()) != 0) {
        return RC::MANIFEST_REPLAY_FAIL;
    }

    if (memcmp(buf + 4, ManifestConfig::versionNum.data(), ManifestConfig::versionNum.size()) != 0) {
        return RC::MANIFEST_REPLAY_FAIL;
    }

    char buf2[ManifestConfig::changeHeadSize];
    size_t size; 

    while ((size = read(file_, buf2, ManifestConfig::changeHeadSize)) == ManifestConfig::changeHeadSize) {
        uint32_t change_len = decodeFix32(buf2);
        uint32_t true_crc = decodeFix32(buf2 + 4);

        char serialize_buf[change_len];
        count = read(file_, serialize_buf, change_len);
        if (count != change_len) {
            return RC::MANIFEST_REPLAY_FAIL;
        }
        uint32_t cur_crc = crc32c::Value(serialize_buf, change_len);
        if (cur_crc != true_crc) {
            return RC::MANIFEST_CRC_CHECK_FAIL; 
        }
        pb::ManifestChangeSet set;
        set.ParseFromArray(serialize_buf, change_len);
        RC result = manifest_->applyChangeSet(set);
        if (result != RC::SUCCESS) {
            return result;
        }
    }

    return RC::SUCCESS;
}

void ManifestFile::buildManifestChange(pb::ManifestChange*& change, pb::ManifestChange_Operation op, int level, uint32_t fd, uint32_t crc) {
    if (op == pb::ManifestChange_Operation_DELETE) {
        change->set_level(level);
        change->set_op(op);
    } else if (op == pb::ManifestChange_Operation_CREATE) {
        change->set_check_sum(crc);
        change->set_id(fd);
        change->set_level(level);
        change->set_op(op);
    } else {
        assert (false);
    }
}

RC ManifestFile::addTableMeta(int level, const std::shared_ptr<Table>& table) {
    pb::ManifestChangeSet set;
    pb::ManifestChange* change = set.add_changes();
    buildManifestChange(change, pb::ManifestChange_Operation_CREATE, level, table->getFD(), table->getCRC());  
    return applyChangeSet(set);
}

pb::ManifestChangeSet ManifestFile::buildChangeSet(CompactionPlan& plan_, std::vector<std::shared_ptr<Table>>& new_tables) {
    pb::ManifestChangeSet set;
    int this_level_num = plan_.this_level_num_;
    int next_level_num = plan_.next_level_num_;
    std::vector<std::shared_ptr<Table>>& this_tables = plan_.this_tables_;
    std::vector<std::shared_ptr<Table>>& next_tables = plan_.next_tables_;
    for (size_t i = 0; i < this_tables.size(); i++) {
        pb::ManifestChange* change = set.add_changes();
        buildManifestChange(change, pb::ManifestChange_Operation_DELETE, this_level_num, this_tables[i]->getFD());
    }

    for (size_t i = 0; i < next_tables.size(); i++) {
        pb::ManifestChange* change = set.add_changes();
        buildManifestChange(change, pb::ManifestChange_Operation_DELETE, next_level_num, next_tables[i]->getFD());
    }

    for (size_t i = 0; i < new_tables.size(); i++) {
        pb::ManifestChange* change = set.add_changes();
        buildManifestChange(change, pb::ManifestChange_Operation_CREATE, next_level_num, new_tables[i]->getFD(), new_tables[i]->getCRC());
    }
    return set;
}

RC ManifestFile::applyChangeSet(const pb::ManifestChangeSet& set) {
    std::lock_guard guard(mutex_);
    RC result = manifest_->applyChangeSet(set);
    if (result != RC::SUCCESS) {
        return result;
    }
    int manifest_creations = manifest_->getCreations();
    int manifest_deletions = manifest_->getDeletions();

    if (manifest_deletions > ManifestConfig::DeleteRewriteThreshold ||
        ManifestConfig::DeleteRewriteRatio* manifest_deletions > (manifest_creations - manifest_deletions)) {
        result = rewriteManifest();
        if (result != RC::SUCCESS) {
            return result;
        }
    } else {
        std::string write_buf;
        std::string serialize_set = set.SerializeAsString();
        uint32_t crc = crc32c::Value(serialize_set.data(), serialize_set.size());
        uint32_t change_len = serialize_set.size();
        encodeFix32(&write_buf, change_len);
        encodeFix32(&write_buf, crc);
        write_buf.append(serialize_set);
        size_t size = write(file_, write_buf.data(), write_buf.size());
        if (size != write_buf.size()) {
            return RC::MANIFEST_WRITE_FILE_FAIL;
        }
        int flag = fsync(file_);
        if (flag != 0) {
            return RC::MANIFEST_FSYNC_FILE_FAIL;
        }
    }
    return RC::SUCCESS;
}

RC ManifestFile::rewriteManifest() {
    std::string rewrite_file_name = opt_->work_dir_ + ManifestConfig::rfileName;
    int file = ::open(rewrite_file_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (file < 0) {
        LOG("unable to open: %s", rewrite_file_name.c_str());
        return RC::MANIFEST_REWRITE_OPEN_FILE_FAIL;
    }
    std::string write_buf;
    // int manifest_creations = manifest_->getCreations();
    // pb::ManifestChange temp;
    // std::cout << temp.ByteSizeLong() << std::endl;
    // write_buf.resize(16);
    write_buf.append(ManifestConfig::magicNum);
    write_buf.append(ManifestConfig::versionNum);
    pb::ManifestChangeSet set;

    for (const auto& iterator: manifest_->getTables()) {
        pb::ManifestChange* change = set.add_changes();
        buildManifestChange(change, pb::ManifestChange_Operation_CREATE, iterator.second.level_, iterator.first, iterator.second.crc_); 
    }
    std::string serialize_set = set.SerializeAsString();
    uint32_t crc = crc32c::Value(serialize_set.data(), serialize_set.size());
    uint32_t change_len = serialize_set.size();
    if (change_len > 0) {
        encodeFix32(&write_buf, change_len);
        encodeFix32(&write_buf, crc);
        write_buf.append(serialize_set);
    }
    size_t size = write(file, write_buf.data(), write_buf.size());
    if (size != write_buf.size()) {
        return RC::MANIFEST_WRITE_FILE_FAIL;
    }
    int flag = fsync(file);
    if (flag != 0) {
        return RC::MANIFEST_FSYNC_FILE_FAIL;
    }
    if (close(file) != 0) {
        return RC::MANIFEST_CLOSE_FILE_FAIL;
    }

    std::string file_name = opt_->work_dir_ + ManifestConfig::fileName; 
    if (rename(rewrite_file_name.data(), file_name.data()) != 0) {
        return RC::MANIFEST_RENAME_FILE_FAIL;
    }
    manifest_->setCreations(set.changes_size());
    manifest_->setDeletions(0);
    
    return RC::SUCCESS;
}

RC ManifestFile::revertToManifest(std::set<uint32_t>& sstable_file_id) {
    std::unordered_map<uint32_t, Manifest::TableManifest>& tables = manifest_->getTables();
    for (const auto& iterator: tables) {
        if (sstable_file_id.contains(iterator.first) == false) {
            return RC::MANIFEST_TABLE_CONTAIN_NONEXIST_SSTABLE;
        }
    }

    for (auto it = sstable_file_id.begin(); it != sstable_file_id.end(); ) {
        if (tables.find(*it) == tables.end()) {
            std::string file_name = Util::filePathJoin(opt_->work_dir_, *it, SSTableConfig::filePostfix);
            if (remove(file_name.data()) != 0) {
                return RC::MANIFEST_REMOVE_FILE_FAIL;
            }
            sstable_file_id.erase(it);
        } else {
            ++it;
        }
    }
    return RC::SUCCESS;
}

std::unordered_map<uint32_t, Manifest::TableManifest>& ManifestFile::getTables(){
    return manifest_->getTables();
}