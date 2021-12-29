#ifndef CODEC_H
#define CODEC_H
#include <nlohmann/json.hpp>

#include "entry.h"
#include "util.h"

// keylen | valuelen | expires_at | key | value | crc32
const size_t WalCodec(const Entry* entry, std::string& codec_entry);

const char* OldWalCodec(const Entry* entry);

#endif