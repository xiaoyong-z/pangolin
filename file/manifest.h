#ifndef MANIFEST_H
#define MANIFEST_H

#include "kv.pb.h"
class Manifest {
    struct TableManifest {
        TableManifest(uint8_t level, uint32_t crc): level_(level), crc_(crc) {}
        uint8_t level_;
        uint32_t crc_;
    };
public:
    Manifest():creations_(0){};

    RC addTableMeta(const pb::ManifestChangeSet& set) {
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
            if (tables_.find(id) != tables.end()) {
                // return RC::
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
            uint32_t level = change.level();
            levels_[level].erase(id);
            tables_.erase(id);
            deletions_++;
        } else {
            assert(false);
        }
        return RC::SUCCESS;
    }

private:
    std::vector<std::unordered_set<uint32_t>> levels_;
    std::unordered_map<uint32_t, TableManifest> tables_;
    int creations_;
    int deletions_;
};

class ManifestFile {
    ManifestFile(MmapFile* mmap_file, std::shared_ptr<Options> opt):
        file_(mmap_file), opt_(opt), manifest_(new Manifest()){}
public:
    static ManifestFile* newManifestFile(const std::shared_ptr<FileOptions>& file_opt, std::shared_ptr<Options> opt) {
        MmapFile* mmap_file = MmapFile::newMmapFile(file_opt->file_name_, file_opt->flag_, file_opt->max_sz_);
        if (mmap_file == nullptr) {
            return nullptr;
        }
        ManifestFile* wal_file = new ManifestFile(mmap_file, opt);
        return wal_file;
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
        manifest_->addTableMeta(set);
        
        // return manifest_->addTableMeta(level, table);
        return RC::SUCCESS;
    }

    std::unique_ptr<MmapFile> file_;
    std::shared_ptr<Options> opt_;
    std::unique_ptr<Manifest> manifest_;
    int deletionsRewriteThreshold_;
    std::mutex mutex_;
};



#endif