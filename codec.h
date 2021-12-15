#ifndef CODEC_H
#define CODEC_H
#include <nlohmann/json.hpp>

#include "entry.h"


using json = nlohmann::json;
void to_json(json& j, const Entry& entry) {
    j = json{{"key: ", entry.key_.ToString()}, {"value: ", entry.value_.ToString()}};
}

const char* WalCodec(const Entry* entry) {
    json j = *entry;
    std::stringstream result;
    result << j;
    return result.str().c_str();
}

#endif