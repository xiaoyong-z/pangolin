#include <string>
#include <vector>
#include <filesystem>
#include "gtest/gtest.h"
#include "db.h"

const int max_num = 10000;
const int max_num2 = 10000;

void clear_dir() {
    std::filesystem::remove_all("/home/parallels/metakv/data/");
    std::filesystem::create_directory("/home/parallels/metakv/data/");
}

TEST(COMPACTION_TEST, basic_test) {
    clear_dir();

    std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 4096, 8192, 1024, 0.01);
    std::shared_ptr<DB> db = std::make_shared<DB>(opt);

    while (true) {
        for (size_t i = 0; i < max_num; i++) {
            std::string key = "key" + std::to_string(i);
            std::string value = "value" + std::to_string(i);
            db->set(key, value);
        }

        // db->scan();

        for (size_t i = 0; i < max_num; i++) {
            std::string key = "key" + std::to_string(i);
            std::string value = "value" + std::to_string(i);
            std::string get_value = db->get(key);
            // std::cout << "get_value : " << get_value << std::endl;
            ASSERT_EQ(get_value.compare(value), 0);
        }
        sleep(3);
        std::cout << "success" << std::endl;
    }
    // sleep(1000);
}

TEST(COMPACTION_TEST, read_level_1_test) {
    clear_dir();

    std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 4096, 8192, 1024, 0.01);
    std::shared_ptr<DB> db = std::make_shared<DB>(opt);

  
    for (size_t i = 0; i < max_num2; i++) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        db->set(key, value);
    }

    std::cout << "start compaction" << std::endl;
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();
    db->compaction();

    std::cout << "end compaction" << std::endl;
    // db->scan();

    for (size_t i = 0; i < max_num2; i++) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        std::string get_value = db->get(key);
        // std::cout << "get_value : " << get_value << std::endl;
        ASSERT_EQ(get_value.compare(value), 0);
    }
}

TEST(DB_TEST, recovery_test) {
    std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 4096, 8192, 1024, 0.01);
    std::shared_ptr<DB> db = std::make_shared<DB>(opt);

    db->scan();

    for (size_t i = 0; i < max_num2; i++) {
        std::string key = "key" + std::to_string(i);
        std::string value = "value" + std::to_string(i);
        std::string get_value = db->get(key);
        // std::cout << "get_value : " << get_value << std::endl;
        ASSERT_EQ(get_value.compare(value), 0);
    }
}

// // TEST(DB_TEST, continue_write_test2) {
// //     std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 4096, 8192, 1024, 0.01);
// //     std::shared_ptr<DB> db = std::make_shared<DB>(opt);

// //     for (size_t i = max_num; i < max_num * 2; i++) {
// //         std::string key = "key" + std::to_string(i);
// //         std::string value = "value" + std::to_string(i);
// //         db->set(key, value);
// //     }

// //     for (size_t i = max_num; i < max_num * 2; i++) {
// //         std::string key = "key" + std::to_string(i);
// //         std::string value = "value" + std::to_string(i);
// //         std::string get_value = db->get(key);
// //         ASSERT_EQ(get_value.compare(value), 0);
// //     }
// // }

// TEST(DB_TEST, update_test) {
//     std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 4096, 8192, 1024, 0.01);
//     std::shared_ptr<DB> db = std::make_shared<DB>(opt);

//     for (size_t i = 0; i < max_num; i++) {
//         std::string key = "key" + std::to_string(i);
//         std::string value = "value" + std::to_string(max_num - i);
//         db->set(key, value);
//     }

//     // db->scan();
    
//     for (size_t i = 0; i < max_num; i++) {
//         std::string key = "key" + std::to_string(i);
//         std::string value = "value" + std::to_string(max_num - i);
//         std::string get_value = db->get(key);
//         // std::cout << "get_value : " << get_value << std::endl;
//         ASSERT_EQ(get_value.compare(value), 0);
//     }
// }

// TEST(DB_TEST, delete_test) {
//     std::shared_ptr<Options> opt = std::make_shared<Options>("/home/parallels/metakv/data/", 4096, 8192, 1024, 0.01);
//     std::shared_ptr<DB> db = std::make_shared<DB>(opt);

//     for (size_t i = 0; i < max_num; i++) {
//         std::string key = "key" + std::to_string(i);
//         db->del(key);
//     }

//     db->scan();
    
//     for (size_t i = 0; i < max_num; i++) {
//         std::string key = "key" + std::to_string(i);
//         std::string get_value = db->get(key);
//         // std::cout << "get_value : " << get_value << std::endl;
//         ASSERT_EQ(get_value.compare(""), 0);
//     }
// }


int main(int argc, char** argv) {
    clear_dir();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
