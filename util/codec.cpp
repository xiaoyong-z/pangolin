#include "codec.h"


// keylen | valuelen | key | meta | value | crc32
const size_t walCodec(const Entry* entry, std::string& codec_entry) {
    encodeFix32(&codec_entry, entry->getKey().size());
    encodeFix32(&codec_entry, entry->getValue().size());
    codec_entry.append(entry->getKey().data(), entry->getKey().size());  
    codec_entry.append(entry->getValue().data(), entry->getValue().size());
    uint32_t crc = crc32c::Value(codec_entry.data(), codec_entry.size());
    encodeFix32(&codec_entry, crc);
    return codec_entry.size();
}