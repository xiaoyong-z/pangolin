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
#include "lsm.h"

TEST(RecoveryTest, BasicTest) {
    std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data", 283, 1024, 1024, 0.01);
    LSM* lsm = LSM::newLSM(opt);

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
    Entry entry3(skey3, svalue3);
    Entry result;

    // ASSERT_EQ(lsm->set(&entry1), RC::SUCCESS);
    ASSERT_EQ(lsm->get(skey1, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value1);


    // ASSERT_EQ(lsm->set(&entry2), RC::SUCCESS);
    ASSERT_EQ(lsm->get(skey2, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value2);

    // ASSERT_EQ(lsm->set(&entry3), RC::SUCCESS);
    ASSERT_EQ(lsm->get(skey3, result), RC::SUCCESS);
    ASSERT_EQ(result.value_, value3);

    Arena::Instance()->free();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
