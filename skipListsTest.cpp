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

// TEST(SkipTest, BasicCRUD) {
//     SkipList<char*, char*> skipList;
    
//     Entry<char*, char*> entry1((char*)"key1", (char*)"value1");
//     ASSERT_EQ(skipList.Contains((char*)"key2"), nullptr);
//     skipList.Insert(std::move(entry1));
//     ASSERT_NE(skipList.Contains((char*)"key1"), nullptr);
//     ASSERT_EQ(skipList.Contains((char*)"key1")->value_, (char*)"value1");

//     Entry<char*, char*> entry2((char*)"key2", (char*)"value2");
//     skipList.Insert(std::move(entry2));
//     ASSERT_NE(skipList.Contains((char*)"key2"), nullptr);
//     ASSERT_EQ(skipList.Contains((char*)"key2")->value_, (char*)"value2");

//     ASSERT_EQ(skipList.Contains((char*)"key3"), nullptr);

//     Entry<char*, char*> entry3((char*)"key1", (char*)"value1 + value2");
//     skipList.Insert(std::move(entry3));
//     ASSERT_NE(skipList.Contains((char*)"key1"), nullptr);
//     ASSERT_EQ(skipList.Contains((char*)"key1")->value_, (char*)"value1 + value2");
// }

TEST(SkipTest, BenchmarkCRUD) {
    SkipList<char*, char*> skipList;
    int maxTime = 10000;
    int maxLen = 20;
    char *key;
    char *value;
    Entry<char*, char*>* entry2;
    for (int i = 0; i < maxTime; i++) {
        key = new char[maxLen];
        value = new char[maxLen];
        char key2[maxLen];
        char value2[maxLen];
        snprintf(key, maxLen, "key%d", i);
        snprintf(value, maxLen, "value%d", i);
        snprintf(key2, maxLen, "key%d", i);
        snprintf(value2, maxLen, "value%d", i);
        entry2 = new Entry<char*, char*>(key , value);
        skipList.Insert(std::move(*entry2));
        free(entry2);
        const Entry<char*, char*>*& entry_ptr;
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

const int size = 10;
std::mutex write_mutex;
void SkipListInsert(SkipList<char*, char*> &skipList, int i, int maxLen){
    for (int j = 0; j < size; j++) {
        char* key = new char[maxLen];
        char* value = new char[maxLen];
        snprintf(key, maxLen, "key%d", i * size + j);
        snprintf(value, maxLen, "value%d", i * size + j);
        Entry<char*, char*>* entry2 = new Entry<char*, char*>(key , value);
        write_mutex.lock();
        skipList.Insert(std::move(*entry2));
        write_mutex.unlock();
        free(entry2);
    }
}

int t2 = 0;

void SkipListContain(SkipList<char*, char*> &skipList, int i, int maxLen){
    for (int j = 0; j < size; j++) {
        char key2[maxLen];
        char value2[maxLen];
        snprintf(key2, maxLen, "key%d", i * size + j);
        snprintf(value2, maxLen, "value%d", i * size + j);
        // printf("key= %s\n", key2);
        if (skipList.Contains(key2) != nullptr) {
            ASSERT_EQ(strcmp(skipList.Contains(key2)->value_, value2), 0);
        } else {
            skipList.Contains(key2);
            t2++;
            if (t2 == 1){
                Entry<char*, char*> entry2(key2 , value2);
                skipList.Insert(std::move(entry2));
                skipList.Contains(key2);
            }
            ASSERT_TRUE(false);
        }
    }
}

TEST(SkipList, Concurrent) {
    SkipList<char*, char*> skipList;
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
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
