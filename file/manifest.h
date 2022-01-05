#ifndef MANIFEST_H
#define MANIFEST_H

#include "kv.pb.h"
class Manifest {
public:
    struct TableManifest {
        TableManifest() {}
        TableManifest(uint8_t level, uint32_t crc): level_(level), crc_(crc) {}
        uint8_t level_;
        uint32_t crc_;
    };

    Manifest():creations_(0){};

    RC applyChangeSet(const pb::ManifestChangeSet& set) {
        int size = set.changes_size();
        RC result;
        for (int i = 0; i < size; i++) {
            const pb::ManifestChange& change = set.changes(i);
            result = applyChange(change);
            if (result != RC::SUCCESS) {
                return result;
            }
        }
        return RC::SUCCESS;
    }

    RC applyChange(const pb::ManifestChange& change) {
        pb::ManifestChange_Operation operation = change.op();
        if (operation == pb::ManifestChange_Operation_CREATE) {
            uint32_t id = change.id();
            if (tables_.find(id) != tables_.end()) {
                return RC::MANIFEST_TABLE_ALREADY_EXIST;
            }
            uint32_t level = change.level();
            uint32_t crc = change.check_sum();
            uint32_t cur_level = levels_.size();
            for (uint32_t i = cur_level; i <= level; i++) {
                levels_.emplace_back(std::unordered_set<uint32_t>());
            }
            levels_[level].emplace(id);
            tables_.emplace(std::make_pair(id, TableManifest(level, crc)));
            creations_++;
        } else if (operation == pb::ManifestChange_Operation_DELETE) {
            uint32_t id = change.id();
            if (tables_.find(id) != tables_.end()) {
                return RC::MANIFEST_TABLE_NOT_EXIST;
            }
            uint32_t level = change.level();
            levels_[level].erase(id);
            tables_.erase(id);
            deletions_++;
        } else {
            return RC::MANIFEST_ILLEAGAL_OPERATION;
        }
        return RC::SUCCESS;
    }

    inline int getCreations() {
        return creations_;
    }

    inline int getDeletions() {
        return deletions_;
    }

    inline void setCreations(int creations) {
        creations_ = creations;
    }

    inline void setDeletions(int deletions) {
        deletions_ = deletions;
    }

    inline std::unordered_map<uint32_t, TableManifest>& getTables() {
        return tables_;
    } 

private:
    std::vector<std::unordered_set<uint32_t>> levels_;
    std::unordered_map<uint32_t, TableManifest> tables_;
    int creations_;
    int deletions_;
};

class ManifestFile {
    ManifestFile(std::shared_ptr<Options> opt): opt_(opt), manifest_(new Manifest()){}
public:
    void setFile(int file) {
        file_ = file;
    }

    static ManifestFile* openManifestFile(std::shared_ptr<Options> opt) {
        ManifestFile* manifestFile = new ManifestFile(opt);
        std::string file_name = opt->work_dir_ + ManifestConfig::fileName;
        int file = open(file_name.c_str(), O_RDONLY, 0666);
        if (file < 0) {
            RC result = manifestFile->rewriteManifest();
            if (result != RC::SUCCESS) {
                delete manifestFile;
                return nullptr;
            }
            file = open(file_name.c_str(), O_RDONLY, 0666);
            assert(file > 0);
            manifestFile->setFile(file);
        } else {
            manifestFile->setFile(file); 
            RC result = manifestFile->replayManifest();
            if (result != RC::SUCCESS) {
                return nullptr;
            }
        }
        return manifestFile;
    }

    RC replayManifest() {
        char buf[ManifestConfig::changeHeadSize];
        size_t count = read(file_, buf, ManifestConfig::changeHeadSize);
        if (count != ManifestConfig::changeHeadSize) {
            return RC::MANIFEST_REPLAY_FAIL;
        }
        
        if (memcmp(buf, ManifestConfig::magicNum.data(), ManifestConfig::magicNum.size()) != 0) {
            return RC::MANIFEST_REPLAY_FAIL;
        }

        if (memcmp(buf + 4, ManifestConfig::versionNum.data(), ManifestConfig::versionNum.size()) != 0) {
            return RC::MANIFEST_REPLAY_FAIL;
        }

        uint32_t change_len = decodeFix32(buf + 8);
        uint32_t true_crc = decodeFix32(buf + 12);

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

        return manifest_->applyChangeSet(set);
    }

    RC addTableMeta(int level, const std::shared_ptr<Table>& table) {
        pb::ManifestChangeSet set;
        pb::ManifestChange* change = set.add_changes();
        change->set_check_sum(table->getCRC());
        change->set_id(table->getFD());
        change->set_level(level);
        change->set_op(pb::ManifestChange_Operation_CREATE);
        return addTableMeta(set);
    }

    RC addTableMeta(const pb::ManifestChangeSet& set) {
        std::lock_guard guard(mutex_);
        RC result = manifest_->applyChangeSet(set);
        if (result != RC::LEVELS_FILE_NOT_OPEN) {
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
            uint32_t change_len = serialize_set.size() + 4;
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

    RC rewriteManifest() {
        std::string rewrite_file_name = opt_->work_dir_ + ManifestConfig::rfileName;
        int fd_ = ::open(rewrite_file_name.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ < 0) {
            LOG("unable to open: %s", rewrite_file_name.c_str());
            return RC::MANIFEST_REWRITE_OPEN_FILE_FAIL;
        }
        std::string write_buf;
        // int manifest_creations = manifest_->getCreations();
        // pb::ManifestChange temp;
        // std::cout << temp.ByteSizeLong() << std::endl;
        write_buf.resize(16);
        write_buf.append(ManifestConfig::magicNum);
        write_buf.append(ManifestConfig::versionNum);
        pb::ManifestChangeSet set;

        for (const auto& iterator: manifest_->getTables()) {
            pb::ManifestChange* change = set.add_changes();
            change->set_id(iterator.first);
            change->set_check_sum(iterator.second.crc_);
            change->set_level(iterator.second.level_);
            change->set_op(pb::ManifestChange_Operation_CREATE);
        }
        std::string serialize_set = set.SerializeAsString();
        uint32_t crc = crc32c::Value(serialize_set.data(), serialize_set.size());
        uint32_t change_len = serialize_set.size();
        encodeFix32(&write_buf, change_len);
        encodeFix32(&write_buf, crc);
        write_buf.append(serialize_set);

        size_t size = write(fd_, write_buf.data(), write_buf.size());
        if (size != write_buf.size()) {
            return RC::MANIFEST_WRITE_FILE_FAIL;
        }
        int flag = fsync(fd_);
        if (flag != 0) {
            return RC::MANIFEST_FSYNC_FILE_FAIL;
        }
        if (close(fd_) != 0) {
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

    RC revertToManifest(std::set<uint32_t>& sstable_file_id) {
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

    std::unordered_map<uint32_t, Manifest::TableManifest>& getTables(){
        return manifest_->getTables();
    }
    
private:
    int file_;
    std::shared_ptr<Options> opt_;
    std::unique_ptr<Manifest> manifest_;
    std::mutex mutex_;
};



#endif