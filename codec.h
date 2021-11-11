#ifndef CODEC_H
#define CODEC_H
#include <nlohmann/json.hpp>

#include "entry.h"


using json = nlohmann::json;
template<typename K, typename V>
void to_json(json& j, const Entry<K, V>& entry) {
    j = json{{"key: ", entry.key_}, {"value: ", entry.value_}};
}

template<typename K, typename V>
const char* WalCodec(const Entry<K, V>* entry) {
    json j = *entry;
    std::stringstream result;
    result << j;
    return result.str().c_str();
}

#endif