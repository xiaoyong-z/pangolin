#include <string>
#include <vector>
#include <filesystem>
#include "gtest/gtest.h"
#include "skipLists.h"
#include "levelsManager.h"
#include "memtable.h"
#include "wal.h"
#include "file.h"
#include "lsm.h"
#include "util.h"

void clear_dir() {
    std::filesystem::remove_all("/home/parallels/metakv/data/");
    std::filesystem::create_directory("/home/parallels/metakv/data/");
}

// TEST(LSM_TEST, basic) {
    
//     // char key2[maxLen];
//     // char value2[maxLen];
//     // snprintf(key, maxLen, "key%d", i);
//     // snprintf(value, maxLen, "value%d", i);
//     // snprintf(key2, maxLen, "key%d", i);
//     // snprintf(value2, maxLen, "value%d", i);
    

//     // std::shared_ptr<Options> opt = std::make_shared<Options>("../work_test", 283, 1024, 1024, 0.01);

//     // for (int i = 0; i < 10; i ++) {
//     //     printf("i = %d\n", i);
        
//     //     std::string key0 = "hello0_12345678";
//     //     std::string key1 = "hello1_12345678";
//     //     std::string key2 = "hello2_12345678";
//     //     std::string key3 = "hello3_12345678";
//     //     std::string key4 = "hello4_12345678";
//     //     std::string key5 = "hello5_12345678";
//     //     std::string key6 = "hello6_12345678";
//     //     std::string key7 = "hello7_12345678";
//     //     std::string value0 = "world0";
//     //     std::string value1 = "world1";
//     //     std::string value2 = "world2";
//     //     std::string value3 = "world3";
//     //     std::string value4 = "world4";
//     //     std::string value5 = "world5";
//     //     std::string value6 = "world6";
//     //     std::string value7 = "world7";
//     //     std::shared_ptr<Entry> entry0 = std::make_shared<Entry>(std::move(key0), std::move(value0));
//     //     std::shared_ptr<Entry> entry1 = std::make_shared<Entry>(std::move(key1), std::move(value1));
//     //     std::shared_ptr<Entry> entry2 = std::make_shared<Entry>(std::move(key2), std::move(value2));
//     //     std::shared_ptr<Entry> entry3 = std::make_shared<Entry>(std::move(key3), std::move(value3));
//     //     std::shared_ptr<Entry> entry4 = std::make_shared<Entry>(std::move(key4), std::move(value4));
//     //     std::shared_ptr<Entry> entry5 = std::make_shared<Entry>(std::move(key5), std::move(value5));
//     //     std::shared_ptr<Entry> entry6 = std::make_shared<Entry>(std::move(key6), std::move(value6));
//     //     std::shared_ptr<Entry> entry7 = std::make_shared<Entry>(std::move(key7), std::move(value7));
        
//     //     std::vector<std::shared_ptr<Entry>> vec;
//     //     vec.push_back(entry0);
//     //     vec.push_back(entry1);
//     //     vec.push_back(entry2);
//     //     vec.push_back(entry3);
//     //     vec.push_back(entry4);
//     //     vec.push_back(entry5);
//     //     vec.push_back(entry6);
//     //     vec.push_back(entry7);

//     //     std::string file_name_ = opt->work_dir_ + "00000" + std::to_string(i) + ".mem";
//     //     std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
//     //     file_opt->file_name_ = file_name_;
//     //     file_opt->dir_ = opt->work_dir_;
//     //     file_opt->flag_ = O_CREAT | O_RDWR;
//     //     file_opt->max_sz_ = int(opt->SSTable_max_sz);
//     //     WALFile* wal_ptr = WALFile::newWALFile(file_opt);
//     //     if (wal_ptr == nullptr) {
//     //         printf("wal file create failed");
//     //         return;
//     //     }
//     //     std::unique_ptr<WALFile> wal(wal_ptr);

//     //     std::unique_ptr<SkipList> sl = std::make_unique<SkipList>();
//     //     MemTable memtable(std::move(wal), std::move(sl));
//     //     for (size_t i = 0; i < vec.size(); i++) {
//     //         memtable.set(vec[i]);
//     //     }
//     //     std::unique_ptr<LevelsManager> level_manager(LevelsManager::newLevelManager(opt));
//     //     RC result = level_manager->flush(memtable);
//     //     if (result != RC::SUCCESS) {
//     //         printf("flush failed");
//     //         return;
//     //     }
//     //     Entry entry;
//     //     result = level_manager->get("hello0_12345678", entry);
//     //     ASSERT_EQ(strcmp(entry.getValue().data(), "world0"), 0);
//     //     result = level_manager->get("hello5_12345678", entry);
//     //     ASSERT_EQ(strcmp(entry.getValue().data(), "world5"), 0);
//     //     result = level_manager->get("hello7_12345678", entry);
//     //     ASSERT_EQ(strcmp(entry.getValue().data(), "world7"), 0);
//     // }
    
//     // Entry<sts>entry2 = new Entry<char*, char*>(key , value);

//     // SkipList<char*, char*> skipList;
//     // int maxTime = 10000;
//     // char *key;
//     // char *value;
//     // Entry<char*, char*>* entry2;
//     // for (int i = 0; i < maxTime; i++) {
        
//     //     skipList.insert(std::move(*entry2));
//     //     free(entry2);
//     //     ASSERT_NE(skipList.contains(key2), nullptr);
//     //     ASSERT_EQ(strcmp(skipList.contains(key2)->value_, value2), 0);
//     // }
    
//     // std::filesystem::remove_all("/home/parallels/metakv/data/");
//     // std::filesystem::create_directory("/home/parallels/metakv/data/");

//     std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 283, 1024, 1024, 0.01);
//     LSM* lsm = LSM::newLSM(opt);

//     std::string key1 = "key1";
//     std::string value1 = "value1";
//     std::string key2 = "key2";
//     std::string value2 = "value2";
//     std::string key3 = "key3";
//     std::string value3 = "value3";
//     Slice skey1(key1);
//     Slice svalue1(value1);
//     Slice skey2(key2);
//     Slice svalue2(value2);
//     Slice skey3(key3);
//     Slice svalue3(value3);
//     Entry entry1(skey1, svalue1);
//     Entry entry2(skey2, svalue2);
//     Entry entry3(skey3, svalue3);
//     Entry result;

//     ASSERT_EQ(lsm->set(&entry1), RC::SUCCESS);
//     ASSERT_EQ(lsm->get(skey1, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value1);


//     ASSERT_EQ(lsm->set(&entry2), RC::SUCCESS);
//     ASSERT_EQ(lsm->get(skey2, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value2);

//     ASSERT_EQ(lsm->set(&entry3), RC::SUCCESS);
//     ASSERT_EQ(lsm->get(skey3, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value3);


//     Entry result2;
//     ASSERT_EQ(lsm->flush(), RC::SUCCESS);
//     // ASSERT_EQ(lsm->get(skey1, result2), RC::SUCCESS);
//     // ASSERT_EQ(result2.value_, value1);


//     Arena::Instance()->free();
// }


// TEST(LSM_TEST, recovery) {
//     // std::filesystem::remove_all("/home/parallels/metakv/data/");
//     // std::filesystem::create_directory("/home/parallels/metakv/data/");

//     std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 283, 1024, 1024, 0.01);
//     LSM* lsm = LSM::newLSM(opt);

//     std::string key1 = "key1";
//     std::string value1 = "value1";
//     std::string key2 = "key2";
//     std::string value2 = "value2";
//     std::string key3 = "key3";
//     std::string value3 = "value3";
//     Slice skey1(key1);
//     Slice svalue1(value1);
//     Slice skey2(key2);
//     Slice svalue2(value2);
//     Slice skey3(key3);
//     Slice svalue3(value3);
//     Entry entry1(skey1, svalue1);
//     Entry entry2(skey2, svalue2);
//     Entry entry3(skey3, svalue3);
//     Entry result;

//     ASSERT_EQ(lsm->get(skey1, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value1);


//     ASSERT_EQ(lsm->get(skey2, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value2);

//     ASSERT_EQ(lsm->get(skey3, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value3);


//     std::string key4 = "key4";
//     std::string value4 = "value4";
//     std::string key5 = "key5";
//     std::string value5 = "value5";
//     std::string key6 = "key6";
//     std::string value6 = "value6";
//     Slice skey4(key4);
//     Slice svalue4(value4);
//     Slice skey5(key5);
//     Slice svalue5(value5);
//     Slice skey6(key6);
//     Slice svalue6(value6);
//     Entry entry4(skey4, svalue4);
//     Entry entry5(skey5, svalue5);
//     Entry entry6(skey6, svalue6);

//     ASSERT_EQ(lsm->set(&entry4), RC::SUCCESS);
//     ASSERT_EQ(lsm->get(skey4, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value4);


//     ASSERT_EQ(lsm->set(&entry5), RC::SUCCESS);
//     ASSERT_EQ(lsm->get(skey5, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value5);

//     ASSERT_EQ(lsm->set(&entry6), RC::SUCCESS);
//     ASSERT_EQ(lsm->get(skey6, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value6);


//     ASSERT_EQ(lsm->flush(), RC::SUCCESS);
//     ASSERT_EQ(lsm->get(skey6, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value6);

//     // Entry result2;
//     // ASSERT_EQ(lsm->flush(), RC::SUCCESS);
//     // // ASSERT_EQ(lsm->get(skey1, result2), RC::SUCCESS);
//     // // ASSERT_EQ(result2.value_, value1);


//     // Arena::Instance()->free();
// }

// TEST(LSM_TEST, recovery2) {
//     // std::filesystem::remove_all("/home/parallels/metakv/data/");
//     // std::filesystem::create_directory("/home/parallels/metakv/data/");

//     std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 283, 1024, 1024, 0.01);
//     LSM* lsm = LSM::newLSM(opt);

//     std::string key4 = "key4";
//     std::string value4 = "value4";
//     std::string key5 = "key5";
//     std::string value5 = "value5";
//     std::string key6 = "key6";
//     std::string value6 = "value6";
//     Slice skey4(key4);
//     Slice svalue4(value4);
//     Slice skey5(key5);
//     Slice svalue5(value5);
//     Slice skey6(key6);
//     Slice svalue6(value6);
//     Entry entry4(skey4, svalue4);
//     Entry entry5(skey5, svalue5);
//     Entry entry6(skey6, svalue6);
//     Entry result;

//     ASSERT_EQ(lsm->get(skey4, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value4);

//     ASSERT_EQ(lsm->get(skey5, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value5);

//     ASSERT_EQ(lsm->get(skey6, result), RC::SUCCESS);
//     ASSERT_EQ(result.getValue(), value6);
// }


int main(int argc, char** argv) {
    clear_dir();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
