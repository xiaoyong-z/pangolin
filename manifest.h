#ifndef MANIFEST_H
#define MANIFEST_H

class Manifest {
    class TableManifest {
        uint8_t level;
        uint32_t crc;
    };

    int creations;
    int deletions;
};

#endif