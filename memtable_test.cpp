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
#include "memtable.h"
typedef uint64_t Key;

TEST(MemTableTest, BasicCRUD) {
    std::shared_ptr<Options> opt = std::make_shared<Options>("../work_test", 283, 1024, 1024, 0.01);
    
    std::string file_name_ = opt->work_dir_ + "00000" + std::to_string(0) + ".mem";
    std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
    file_opt->file_name_ = file_name_;
    file_opt->dir_ = opt->work_dir_;
    file_opt->flag_ = O_CREAT | O_RDWR;
    file_opt->max_sz_ = int(opt->ssTable_max_sz_);

    WALFile* wal_ptr = WALFile::NewWALFile(file_opt);
    if (wal_ptr == nullptr) {
        printf("wal file create failed");
        return;
    }
    std::unique_ptr<WALFile> wal(wal_ptr);

    std::unique_ptr<SkipList> sl = std::make_unique<SkipList>();
    MemTable memtable(std::move(wal), std::move(sl));

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

    ASSERT_EQ(memtable.set(&entry1), RC::SUCCESS);
    ASSERT_EQ(memtable.get(skey1, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value1);


    ASSERT_EQ(memtable.set(&entry2), RC::SUCCESS);
    ASSERT_EQ(memtable.get(skey2, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value2);

    ASSERT_EQ(memtable.set(&entry3), RC::SUCCESS);
    ASSERT_EQ(memtable.get(skey1, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value3);

    Arena::Instance()->free();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
