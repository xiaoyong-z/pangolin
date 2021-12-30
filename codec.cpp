#include "codec.h"


// keylen | valuelen | expires_at | key | value | crc32
const size_t walCodec(const Entry* entry, std::string& codec_entry) {
    encodeFix32(&codec_entry, entry->key_.size());
    encodeFix32(&codec_entry, entry->value_.size());
    encodeFix64(&codec_entry, entry->expires_at_);
    codec_entry.append(entry->key_.data(), entry->key_.size());
    codec_entry.append(entry->value_.data(), entry->value_.size());
    uint32_t crc = crc32c::Value(codec_entry.data(), codec_entry.size());
    encodeFix32(&codec_entry, crc);
    return codec_entry.size();
}


using json = nlohmann::json;
void to_json(json& j, const Entry& entry) {
    j = json{{"key: ", entry.key_.ToString()}, {"value: ", entry.value_.ToString()}};
}

const char* oldWalCodec(const Entry* entry) {
    json j = *entry;
    std::stringstream result;
    result << j;
    return result.str().c_str();
}