#ifndef MANIFEST_H
#define MANIFEST_H


class Manifest {
    struct TableManifest {
        uint8_t level;
        uint32_t crc;
    };
     
    std::vector<std::unordered_set<uint64_t>> levels;
    std::unordered_map<uint64_t, TableManifest> tables;
    int creations;
    int deletions;
};

#endif