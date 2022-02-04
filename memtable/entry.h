#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>

#include "slice.h"
#include "util.h"
struct ValueStruct {
    ValueStruct(char* value, uint32_t value_len): value_(value, value_len){}
    Slice value_;
};

// Slice的优点： 1. 复制开销很小，不用专门去使用移动语义。
//              2. 可以直接从char*转过来，对于string，从char*转过来会进行逐个复制
//        缺点： 1. 底下的内存管理需要自己手工管理
struct Entry {
    ~Entry(){};
    
    Entry(): type_(kTypeValue) {};
    
    Entry(Slice& key, Slice& value): key_(key), value_(value), type_(kTypeValue) {};

    void reset(const Slice& key, const ValueStruct& value){
        key_ = key;
        value_ = value.value_;
    }

    void reset(const Slice& key, const Slice& value){
        key_ = key;
        value_ = value;
    }

    uint32_t estimateWalEntrySize() {
        return key_.size() + value_.size() + CRC_SIZE_LEN + KEY_VALUE_LEN;
    }

    void setKey(const Slice& key) {
        key_ = key;
    }

    void setVersionNumAndType(uint32_t version_num, ValueType type = kTypeValue) {
        version_num_ = version_num;
        type_ = type;
        char* addr = new char[key_.size() + META_SIZE];
        memcpy(addr, key_.data(), key_.size());
        Util::encodeVersionNumType(addr + key_.size(), version_num, type);
        key_.reset(addr, key_.size() + META_SIZE);
    }

    void resetKey(char* addr, size_t size) {
        key_.reset(addr, size);
        Util::decodeVersionNumType(addr + size - META_SIZE, version_num_, type_);
    }

    void resetValue(const char* addr, size_t size) {
        value_.reset(addr, size);
    }

    inline Slice& getKey() const {
        return key_;
    }

    inline Slice& getValue() const {
        return value_;
    }

    inline uint32_t getVersionNum() const {
        return version_num_;
    }

    inline ValueType getType() const {
        return type_;
    }

private:
    mutable Slice key_;
    mutable Slice value_;

    // derived from key_
    uint32_t version_num_;
    ValueType type_;
};
#endif
