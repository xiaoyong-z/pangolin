#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>

#include "slice.h"
struct ValueStruct {
    ValueStruct(char* value, uint32_t value_len, uint64_t expire_at): value_(value, value_len), expires_at_(expire_at) {}
    Slice value_;
    uint64_t expires_at_;
};

// Slice的优点： 1. 复制开销很小，不用专门去使用移动语义。
//              2. 可以直接从char*转过来，对于string，从char*转过来会进行逐个复制
//        缺点： 1. 底下的内存管理需要自己手工管理
struct Entry {
    ~Entry(){};
    
    Entry(){};
    
    Entry(Slice& key, Slice& value): key_(key), value_(value), expires_at_(0) {};

    void reset(const Slice& key, const ValueStruct& value){
        key_ = key;
        value_ = value.value_;
        expires_at_ = value.expires_at_;
    }

    void reset(const Slice& key, const Slice& value){
        key_ = key;
        value_ = value;
    }


    // Entry(Entry && another): key_(std::move(another.key_)), value_(std::move(another.value_)){};

    // Entry& operator=(Entry && another) {
    //     key_ = std::move(another.key_);
    //     value_ = std::move(another.value_);
    //     return *this;
    // }

    Slice key_;
    Slice value_;
    uint64_t expires_at_;
};



// template<>
// struct Entry<char*, char*> {
//     ~Entry() {
//         if (key_) {
//             free(key_);
//             key_ = nullptr;
//         }
//         if (value_) {
//             free(value_);
//             value_ = nullptr;
//         }
//     };
//     Entry(){
//         key_ = nullptr;
//         value_ = nullptr;
//     };
//     Entry(char* key, char* value): key_(key), value_(value){};
//     char* key_;
//     char* value_;
//     uint64_t expires_at_;
// };

// class TestMove{
// public:

//     TestMove() {
//         std::cout << "zero constructor" << std::endl;
//     }

//     ~TestMove() {
//         std::cout << "deconstructor called" << std::endl;
//     }

//     TestMove(double score, int a) {
//         std::cout << "default constructor" << std::endl;
//         score_ = score;
//         a_ = a;
//     }

//     TestMove(TestMove&& another) {
//         std::cout << "move constrcutor" << std::endl;
//         score_ = another.score_;
//         a_ = another.a_;
//     }

//     TestMove& operator=(TestMove&& str) {
//         std::cout << "move assignment" << std::endl;
//         score_ = str.score_;
//         a_ = str.a_;
//         return *this;
//     }

//     bool operator==(const TestMove& another) const{
//         return score_ == another.score_ && a_ == another.a_;
//     }


//     TestMove(const TestMove& another) = delete;

//     TestMove& operator=(const TestMove& str) = delete;

//     // TestMove(const TestMove& another) {
//     //     std::cout << "copy constructor" << std::endl;
//     //     score_ = another.score_;
//     //     a_ = another.a_;
//     // }

//     // TestMove& operator=(const TestMove& str) {
//     //     std::cout << "copy assignment" << std::endl;
//     //     score_ = str.score_;
//     //     a_ = str.a_;
//     //     return *this;
//     // }

//     double score_;
//     int a_;
// };

// using Entry = Entry<std::string, std::string>;
#endif
