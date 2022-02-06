#ifndef MANIFEST_H
#define MANIFEST_H

#include <sys/stat.h>
#include "kv.pb.h"
#include "compactionPlan.h"
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
            if (tables_.find(id) == tables_.end()) {
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

class CompactionPlan;
class ManifestFile {
    ManifestFile(std::shared_ptr<Options> opt);
public:
    void setFile(int file);
    ~ManifestFile();
    static ManifestFile* openManifestFile(std::shared_ptr<Options> opt);
    RC replayManifest();
    RC addTableMeta(int level, const std::shared_ptr<Table>& table);
    static pb::ManifestChangeSet buildChangeSet(CompactionPlan& plan_, std::vector<std::shared_ptr<Table>>& new_tables);
    RC applyChangeSet(const pb::ManifestChangeSet& set);
    RC rewriteManifest();
    RC revertToManifest(std::set<uint32_t>& sstable_file_id);
    std::unordered_map<uint32_t, Manifest::TableManifest>& getTables();
    
private:
    static void buildManifestChange(pb::ManifestChange*& change, pb::ManifestChange_Operation op, int level, uint32_t fd = 0, uint32_t crc = 0);

    int file_;
    std::shared_ptr<Options> opt_;
    std::unique_ptr<Manifest> manifest_;
    std::mutex mutex_;
};
#endif