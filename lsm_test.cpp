// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "skipLists.h"
#include "levels.h"
#include "memtable.h"
#include "wal.h"
#include "file.h"
#include "lsm.h"
#include "util.h"

TEST(LSM_TEST, basic) {
    
    // char key2[maxLen];
    // char value2[maxLen];
    // snprintf(key, maxLen, "key%d", i);
    // snprintf(value, maxLen, "value%d", i);
    // snprintf(key2, maxLen, "key%d", i);
    // snprintf(value2, maxLen, "value%d", i);
    

    // std::shared_ptr<Options> opt = std::make_shared<Options>("../work_test", 283, 1024, 1024, 0.01);

    // for (int i = 0; i < 10; i ++) {
    //     printf("i = %d\n", i);
        
    //     std::string key0 = "hello0_12345678";
    //     std::string key1 = "hello1_12345678";
    //     std::string key2 = "hello2_12345678";
    //     std::string key3 = "hello3_12345678";
    //     std::string key4 = "hello4_12345678";
    //     std::string key5 = "hello5_12345678";
    //     std::string key6 = "hello6_12345678";
    //     std::string key7 = "hello7_12345678";
    //     std::string value0 = "world0";
    //     std::string value1 = "world1";
    //     std::string value2 = "world2";
    //     std::string value3 = "world3";
    //     std::string value4 = "world4";
    //     std::string value5 = "world5";
    //     std::string value6 = "world6";
    //     std::string value7 = "world7";
    //     std::shared_ptr<Entry> entry0 = std::make_shared<Entry>(std::move(key0), std::move(value0));
    //     std::shared_ptr<Entry> entry1 = std::make_shared<Entry>(std::move(key1), std::move(value1));
    //     std::shared_ptr<Entry> entry2 = std::make_shared<Entry>(std::move(key2), std::move(value2));
    //     std::shared_ptr<Entry> entry3 = std::make_shared<Entry>(std::move(key3), std::move(value3));
    //     std::shared_ptr<Entry> entry4 = std::make_shared<Entry>(std::move(key4), std::move(value4));
    //     std::shared_ptr<Entry> entry5 = std::make_shared<Entry>(std::move(key5), std::move(value5));
    //     std::shared_ptr<Entry> entry6 = std::make_shared<Entry>(std::move(key6), std::move(value6));
    //     std::shared_ptr<Entry> entry7 = std::make_shared<Entry>(std::move(key7), std::move(value7));
        
    //     std::vector<std::shared_ptr<Entry>> vec;
    //     vec.push_back(entry0);
    //     vec.push_back(entry1);
    //     vec.push_back(entry2);
    //     vec.push_back(entry3);
    //     vec.push_back(entry4);
    //     vec.push_back(entry5);
    //     vec.push_back(entry6);
    //     vec.push_back(entry7);

    //     std::string file_name_ = opt->work_dir_ + "00000" + std::to_string(i) + ".mem";
    //     std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
    //     file_opt->file_name_ = file_name_;
    //     file_opt->dir_ = opt->work_dir_;
    //     file_opt->flag_ = O_CREAT | O_RDWR;
    //     file_opt->max_sz_ = int(opt->ssTable_max_sz_);
    //     WALFile* wal_ptr = WALFile::NewWALFile(file_opt);
    //     if (wal_ptr == nullptr) {
    //         printf("wal file create failed");
    //         return;
    //     }
    //     std::unique_ptr<WALFile> wal(wal_ptr);

    //     std::unique_ptr<SkipList> sl = std::make_unique<SkipList>();
    //     MemTable memtable(std::move(wal), std::move(sl));
    //     for (size_t i = 0; i < vec.size(); i++) {
    //         memtable.set(vec[i]);
    //     }
    //     std::unique_ptr<LevelManager> level_manager(LevelManager::newLevelManager(opt));
    //     RC result = level_manager->flush(memtable);
    //     if (result != RC::SUCCESS) {
    //         printf("flush failed");
    //         return;
    //     }
    //     Entry entry;
    //     result = level_manager->get("hello0_12345678", entry);
    //     ASSERT_EQ(strcmp(entry.value_.data(), "world0"), 0);
    //     result = level_manager->get("hello5_12345678", entry);
    //     ASSERT_EQ(strcmp(entry.value_.data(), "world5"), 0);
    //     result = level_manager->get("hello7_12345678", entry);
    //     ASSERT_EQ(strcmp(entry.value_.data(), "world7"), 0);
    // }
    
    // Entry<sts>entry2 = new Entry<char*, char*>(key , value);

    // SkipList<char*, char*> skipList;
    // int maxTime = 10000;
    // char *key;
    // char *value;
    // Entry<char*, char*>* entry2;
    // for (int i = 0; i < maxTime; i++) {
        
    //     skipList.Insert(std::move(*entry2));
    //     free(entry2);
    //     ASSERT_NE(skipList.Contains(key2), nullptr);
    //     ASSERT_EQ(strcmp(skipList.Contains(key2)->value_, value2), 0);
    // }
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
