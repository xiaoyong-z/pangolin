#include <string>
#include <vector>
#include <filesystem>
#include "gtest/gtest.h"
#include "db.h"

void clear_dir() {
    std::filesystem::remove_all("/home/parallels/metakv/data/");
    std::filesystem::create_directory("/home/parallels/metakv/data/");
}

TEST(DB_TEST, basic) {
    std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 1024, 4096, 1024, 0.01);
    std::shared_ptr<DB> db = std::make_shared<DB>(opt);

    std::string key1 = "key1";
    std::string value1 = "value1";
    db->set(key1, value1);

    ASSERT_EQ(db->get(key1).compare(value1), 0);

    std::string key2 = "key2";
    std::string value2 = "value2";
    std::string key3 = "key3";
    std::string value3 = "value3";

    db->set(key2, value2);
    db->set(key3, value3);
    
    ASSERT_EQ(db->get(key2).compare(value2), 0);
    ASSERT_EQ(db->get(key3).compare(value3), 0);
}


TEST(DB_TEST, recovery) {
    std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 1024, 4096, 1024, 0.01);
    std::shared_ptr<DB> db = std::make_shared<DB>(opt);

    std::string key1 = "key1";
    std::string value1 = "value1";
    std::string key2 = "key2";
    std::string value2 = "value2";
    std::string key3 = "key3";
    std::string value3 = "value3";

    ASSERT_EQ(db->get(key1).compare(value1), 0);
    ASSERT_EQ(db->get(key2).compare(value2), 0);
    ASSERT_EQ(db->get(key3).compare(value3), 0);
}

TEST(DB_TEST, recovery2) {
    // std::filesystem::remove_all("/home/parallels/metakv/data/");
    // std::filesystem::create_directory("/home/parallels/metakv/data/");

    // std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 283, 1024, 1024, 0.01);
    // LSM* lsm = LSM::newLSM(opt);

    // std::string key4 = "key4";
    // std::string value4 = "value4";
    // std::string key5 = "key5";
    // std::string value5 = "value5";
    // std::string key6 = "key6";
    // std::string value6 = "value6";
    // Slice skey4(key4);
    // Slice svalue4(value4);
    // Slice skey5(key5);
    // Slice svalue5(value5);
    // Slice skey6(key6);
    // Slice svalue6(value6);
    // Entry entry4(skey4, svalue4);
    // Entry entry5(skey5, svalue5);
    // Entry entry6(skey6, svalue6);
    // Entry result;

    // ASSERT_EQ(lsm->get(skey4, result), RC::SUCCESS);
    // ASSERT_EQ(result.value_, value4);

    // ASSERT_EQ(lsm->get(skey5, result), RC::SUCCESS);
    // ASSERT_EQ(result.value_, value5);

    // ASSERT_EQ(lsm->get(skey6, result), RC::SUCCESS);
    // ASSERT_EQ(result.value_, value6);
}


int main(int argc, char** argv) {
    clear_dir();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
