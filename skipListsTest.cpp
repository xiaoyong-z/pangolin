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
#include "skipLists.cpp"

typedef uint64_t Key;

std::string RandString(int len) {
    char* bytes = new char[len];
    for (int i = 0; i < len; i++) {
        char b = rand() % 256;
        bytes[i] = b;
    }
    return std::string(bytes);
}

TEST(SkipTest, TestMove) {
    TestMove a(1.2, 1);
    TestMove b(2.3, 1);
    TestMove c(1.2, 2);

    std::cout << "generate" << std::endl;
    Entry<TestMove, TestMove> entry1(std::move(a), std::move(b));

    SkipList<TestMove, TestMove> skipList;


    ASSERT_EQ(skipList.Contains(c), nullptr);

    skipList.Insert(std::move(entry1));

    TestMove d(2.3, 1);

    ASSERT_NE(skipList.Contains(a), nullptr);
    ASSERT_EQ(skipList.Contains(a)->value_, d);

    TestMove e(1.2, 1);
    TestMove f(2.111, 10);

    Entry<TestMove, TestMove> entry2(std::move(e), std::move(f));
    std::cout << "Update before" << std::endl;
    skipList.Insert(std::move(entry2));
    std::cout << "Update After" << std::endl;
    
}

TEST(SkipTest, BasicCRUD) {
    SkipList<char*, char*> skipList;
    
    Entry<char*, char*> entry1((char*)"key1", (char*)"value1");
    ASSERT_EQ(skipList.Contains((char*)"key2"), nullptr);
    skipList.Insert(std::move(entry1));
    ASSERT_NE(skipList.Contains((char*)"key1"), nullptr);
    ASSERT_EQ(skipList.Contains((char*)"key1")->value_, (char*)"value1");

    Entry<char*, char*> entry2((char*)"key2", (char*)"value2");
    skipList.Insert(std::move(entry2));
    ASSERT_NE(skipList.Contains((char*)"key2"), nullptr);
    ASSERT_EQ(skipList.Contains((char*)"key2")->value_, (char*)"value2");

    ASSERT_EQ(skipList.Contains((char*)"key3"), nullptr);

    Entry<char*, char*> entry3((char*)"key1", (char*)"value1 + value2");
    skipList.Insert(std::move(entry3));
    ASSERT_NE(skipList.Contains((char*)"key1"), nullptr);
    ASSERT_EQ(skipList.Contains((char*)"key1")->value_, (char*)"value1 + value2");
}

TEST(SkipTest, BenchmarkCRUD) {
    SkipList<char*, char*> skipList;
    int maxTime = 100000;
    int maxLen = 1000;
    for (int i = 0; i < maxTime; i++) {
        char key[maxLen];
        char value[maxLen];
        char key2[maxLen];
        char value2[maxLen];
        snprintf(key, maxLen, "key%d", i);
        snprintf(key2, maxLen, "key%d", i);
        snprintf(value, maxLen, "value%d", i);
        snprintf(value2, maxLen, "value%d", i);
        Entry<char*, char*> entry2(key , value);
        skipList.Insert(std::move(entry2));
        ASSERT_NE(skipList.Contains(key2), nullptr);
        ASSERT_EQ(strcmp(skipList.Contains(key2)->value_, value2), 0);
    }
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

void SkipListInsert(SkipList<char*, char*> &skipList, int i, int maxLen){
    char* key = new char[maxLen];
    char* value = new char[maxLen];
    snprintf(key, maxLen, "key%d", i);
    snprintf(value, maxLen, "value%d", i);
    Entry<char*, char*> entry2(key , value);
    skipList.Insert(std::move(entry2));
}

void SkipListContain(SkipList<char*, char*> &skipList, int i, int maxLen){
    char key2[maxLen];
    char value2[maxLen];
    snprintf(key2, maxLen, "key%d", i);
    snprintf(value2, maxLen, "value%d", i);
    ASSERT_NE(skipList.Contains(key2), nullptr);
    ASSERT_EQ(strcmp(skipList.Contains(key2)->value_, value2), 0);
}

TEST(SkipList, Concurrent) {
    SkipList<char*, char*> skipList;
    int maxTime = 40000;
    int maxLen = 100;
    
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
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
