#include <atomic>
#include <set>
#include <string>
#include <memory>
#include <condition_variable>
#include <thread>
#include "gtest/gtest.h"
#include "memtable.h"
typedef uint64_t Key;

TEST(MemTableTest, BasicTest) {
    std::shared_ptr<Options> opt = std::make_shared<Options>("../work_test", 283, 1024, 1024, 0.01);
    
    std::string file_name_ = opt->work_dir_ + "00000" + std::to_string(0) + ".mem";
    std::shared_ptr<FileOptions> file_opt = std::make_shared<FileOptions>();
    file_opt->file_name_ = file_name_;
    file_opt->dir_ = opt->work_dir_;
    file_opt->flag_ = O_CREAT | O_RDWR;
    file_opt->max_sz_ = int(opt->SSTable_max_sz);

    WALFile* wal_ptr = WALFile::newWALFile(file_opt);
    if (wal_ptr == nullptr) {
        printf("wal file create failed");
        return;
    }
    std::unique_ptr<WALFile> wal(wal_ptr);

    std::unique_ptr<SkipList> sl = std::make_unique<SkipList>();
    MemTable memtable(std::move(wal), std::move(sl));

    for (int i = 0; i < 50; i++) {
        std::string key = "key" + std::to_string(i);
        encodeFix32(&key, i);
        std::string value = "value" + std::to_string(i);
        encodeFix32(&value, i);

        Slice skey1(key);
        Slice svalue1(value);
        Entry entry1(skey1, svalue1);

        Entry result;

        ASSERT_EQ(memtable.set(&entry1), RC::SUCCESS);
        // ASSERT_EQ(memtable.get(skey1, result), RC::SUCCESS);
        // ASSERT_EQ(result.getValue(), value);
    }

    memtable.scan();

    // Arena::Instance()->free();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
