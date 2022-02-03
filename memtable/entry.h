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
    
    Entry(){};
    
    Entry(Slice& key, Slice& value): key_(key), value_(value) {};

    void reset(const Slice& key, const ValueStruct& value){
        key_ = key;
        value_ = value.value_;
    }

    void reset(const Slice& key, const Slice& value){
        key_ = key;
        value_ = value;
    }

        uint32_t estimateWalEntrySize() {
        return key_.size() + value_.size() + 8 + 8 + CRC_SIZE_LEN;
    }

    void setKey(const Slice& key) {
        key_ = key;
    }

    Slice key_;
    Slice value_;
};
#endif
