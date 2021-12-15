// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <atomic>
#include <set>
#include <string>
#include <memory>
#include <condition_variable>
#include <thread>
#include "gtest/gtest.h"
#include "skipLists.h"
#include "arena.h"
typedef uint64_t Key;

std::string RandString(int len) {
    char* bytes = new char[len];
    for (int i = 0; i < len; i++) {
        char b = rand() % 256;
        bytes[i] = b;
    }
    return std::string(bytes);
}

// TEST(SkipTest, TestMove) {
//     TestMove a(1.2, 1);
//     TestMove b(2.3, 1);
//     TestMove c(1.2, 2);

//     std::cout << "generate" << std::endl;
//     Entry<TestMove, TestMove> entry1(std::move(a), std::move(b));

//     SkipList<TestMove, TestMove> skipList;


//     const Entry<TestMove, TestMove>* result;
//     ASSERT_EQ(skipList.Contains(c, result), RC::SKIPLIST_NOT_FOUND);

//     skipList.Insert(std::move(entry1));

//     TestMove d(2.3, 1);

//     ASSERT_EQ(skipList.Contains(a, result), RC::SUCCESS);
//     ASSERT_EQ(result->value_, d);

//     TestMove e(1.2, 1);
//     TestMove f(2.111, 10);

//     Entry<TestMove, TestMove> entry2(std::move(e), std::move(f));
//     std::cout << "Update before" << std::endl;
//     skipList.Insert(std::move(entry2));
//     std::cout << "Update After" << std::endl;
    
// }

TEST(SkipTest, BasicCRUD) {
    SkipList skipList;
    std::string key1 = "key1";
    std::string value1 = "value1";
    std::string key2 = "key2";
    std::string value2 = "value2";
    std::string key3 = "key3";
    std::string value3 = "value3";
    Slice skey1(key1);
    Slice svalue1(value1);
    Slice skey2(key2);
    Slice svalue2(value2);
    Slice skey3(key3);
    Slice svalue3(value3);
    Entry entry1(skey1, svalue1);
    Entry entry2(skey2, svalue2);
    Entry entry3(skey1, svalue3);
    Entry result;

    ASSERT_EQ(skipList.Contains(skey2, result), RC::SKIPLIST_NOT_FOUND);

    skipList.Insert(&entry1);
    ASSERT_EQ(skipList.Contains(skey1, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value1);

    skipList.Insert(&entry2);
    ASSERT_EQ(skipList.Contains(skey2, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value2);

    skipList.Insert(&entry3);
    ASSERT_EQ(skipList.Contains(skey1, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value3);

    Arena::Instance()->free();
}

TEST(SkipTest, BenchmarkCRUD) {
    SkipList skipList;
    int maxTime = 10000;
    
    char* key_ptr;
    char* value_ptr;
    Entry entry;
    Entry result;
    for (int i = 0; i < maxTime; i++) {
        std::string str = "key" + std::to_string(i);
        key_ptr = new char[str.size()];
        memmove(key_ptr, str.data(), str.size());
        Slice key(key_ptr, str.size());
        
        str = "value" + std::to_string(i);
        value_ptr = new char[str.size()];
        memmove(value_ptr, str.data(), str.size());
        Slice value(value_ptr, str.size());

        entry.reset(key, value);
        skipList.Insert(&entry);
        // RC rc = skipList.Contains(key, result);
        ASSERT_EQ(skipList.Contains(key, result), RC::SUCCESS);
        ASSERT_EQ(result.value_.compare(value), 0);
        delete[] key_ptr;
        delete[] value_ptr;
    }
    Arena::Instance()->free();
}

class WaitGroup {
public:
    WaitGroup() {
        counter.store(0);
    }
    void Add(int incr = 1) { counter += incr; }
    void Done() { 
        if (--counter <= 0) {
            printf("counter %d\n", counter.load());
            cond.notify_all();
        }
        
    }
    void Wait() {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [&] { return counter <= 0; });
    }

private:
    std::mutex mutex;
    std::atomic<int> counter;
    std::condition_variable cond;
};

const int size = 100;
std::mutex write_mutex;
void SkipListInsert(SkipList &skipList, int i, int maxLen){
    char* key_ptr;
    char* value_ptr;
    Entry entry;
    Entry result;

    for (int j = 0; j < size; j++) {
        std::string str = "key" + std::to_string(i * size + j);
        key_ptr = new char[str.size()];
        memmove(key_ptr, str.data(), str.size());
        Slice key(key_ptr, str.size());
        
        str = "value" + std::to_string(i * size + j);
        value_ptr = new char[str.size()];
        memmove(value_ptr, str.data(), str.size());
        Slice value(value_ptr, str.size());

        entry.reset(key, value);
        write_mutex.lock();
        skipList.Insert(&entry);
        write_mutex.unlock();
        delete[] key_ptr;
        delete[] value_ptr;
    }
}

int t2 = 0;

void SkipListContain(SkipList &skipList, int i, int maxLen){
    for (int j = 0; j < size; j++) {
        std::string str = "key" + std::to_string(i * size + j);
        char* key_ptr = new char[str.size()];
        memmove(key_ptr, str.data(), str.size());
        Slice key(key_ptr, str.size());
        
        str = "value" + std::to_string(i * size + j);
        char* value_ptr = new char[str.size()];
        memmove(value_ptr, str.data(), str.size());
        Slice value(value_ptr, str.size());
        Entry result;
        ASSERT_EQ (skipList.Contains(key, result), RC::SUCCESS);
        ASSERT_EQ (result.value_.compare(value), 0);
        delete[] key_ptr;
        delete[] value_ptr;
    }
}

TEST(SkipList, Concurrent) {
    SkipList skipList;
    int maxTime = 100;
    int maxLen = 30;
    
    std::thread mythread[maxTime];
    for (int i = 0; i < maxTime; i++) {
        mythread[i] = std::thread(SkipListInsert, std::ref(skipList), i, maxLen);
    }
    for (int i = 0; i < maxTime; i++) {
        mythread[i].join();
    }
    for (int i = 0; i < maxTime; i++) {
        mythread[i] = std::thread(SkipListContain, std::ref(skipList), i, maxLen);
    }
    for (int i = 0; i < maxTime; i++) {
        mythread[i].join();
    }
    Arena::Instance()->free();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
