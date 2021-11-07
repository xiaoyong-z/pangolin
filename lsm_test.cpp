// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#include <string>

#include "gtest/gtest.h"
#include "skipLists.h"
#include "memtable.cpp"
#include "wal.h"
#include "file.h"
#include "lsm.h"
TEST(LSM_TEST, basic) {
    
    // char key2[maxLen];
    // char value2[maxLen];
    // snprintf(key, maxLen, "key%d", i);
    // snprintf(value, maxLen, "value%d", i);
    // snprintf(key2, maxLen, "key%d", i);
    // snprintf(value2, maxLen, "value%d", i);
    std::string key0 = "hello0_12345678";
    std::string key1 = "hello1_12345678";
    std::string key2 = "hello2_12345678";
    std::string key3 = "hello3_12345678";
    std::string key4 = "hello4_12345678";
    std::string key5 = "hello5_12345678";
    std::string key6 = "hello6_12345678";
    std::string key7 = "hello7_12345678";
    std::string value0 = "world0";
    std::string value1 = "world1";
    std::string value2 = "world2";
    std::string value3 = "world3";
    std::string value4 = "world4";
    std::string value5 = "world5";
    std::string value6 = "world6";
    std::string value7 = "world7";
    Entry<std::string, std::string>* entry0 = new Entry<std::string, std::string>(std::move(key0), std::move(value0));
    Entry<std::string, std::string>* entry1 = new Entry<std::string, std::string>(std::move(key1), std::move(value1));
    Entry<std::string, std::string>* entry2 = new Entry<std::string, std::string>(std::move(key2), std::move(value2));
    Entry<std::string, std::string>* entry3 = new Entry<std::string, std::string>(std::move(key3), std::move(value3));
    Entry<std::string, std::string>* entry4 = new Entry<std::string, std::string>(std::move(key4), std::move(value4));
    Entry<std::string, std::string>* entry5 = new Entry<std::string, std::string>(std::move(key5), std::move(value5));
    Entry<std::string, std::string>* entry6 = new Entry<std::string, std::string>(std::move(key6), std::move(value6));
    Entry<std::string, std::string>* entry7 = new Entry<std::string, std::string>(std::move(key7), std::move(value7));
    Options opt{
        "../work_test", 
        283,
        1024,
        1024,
        0.01
    };

    std::string file_name_ = opt.work_dir_ + "000001.mem";
    FileOptions file_opt;
    file_opt.file_name_ = file_name_;
    file_opt.dir_ = opt.work_dir_;
    file_opt.flag_ = O_CREAT | O_RDWR;
    file_opt.max_sz_ = int(opt.ssTable_max_sz_);
    std::unique_ptr<WALFile> wal = std::make_unique<WALFile>(file_opt);
    std::unique_ptr<STRSkipList> sl = std::make_unique<STRSkipList>();
    MemTable memtable(std::move(wal), std::move(sl));
    

    
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
