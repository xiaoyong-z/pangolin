#ifndef CODEC_H
#define CODEC_H
#include <nlohmann/json.hpp>

#include "entry.h"
#include "util.h"

// keylen | valuelen | expires_at | key | value | crc32
const size_t walCodec(const Entry* entry, std::string& codec_entry);

const char* oldWalCodec(const Entry* entry);

#endif