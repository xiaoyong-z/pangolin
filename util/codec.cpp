#include "codec.h"


// keylen | valuelen | key | value | crc32
const size_t walCodec(const Entry* entry, std::string& codec_entry) {
    encodeFix32(&codec_entry, entry->key_.size());
    encodeFix32(&codec_entry, entry->value_.size());
    codec_entry.append(entry->key_.data(), entry->key_.size());
    codec_entry.append(entry->value_.data(), entry->value_.size());
    uint32_t crc = crc32c::Value(codec_entry.data(), codec_entry.size());
    encodeFix32(&codec_entry, crc);
    return codec_entry.size();
}