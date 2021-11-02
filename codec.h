#include<iostream>

template<typename K, typename V>
struct Entry {
    ~Entry(){};
    
    Entry(){};
    
    Entry(K&& key, V&& value): key_(std::move(key)), value_(std::move(value)){};

    Entry(Entry && another): key_(std::move(another.key_)), value_(std::move(another.value_)){};

    Entry& operator=(Entry && another) {
        key_ = std::move(another.key_);
        value_ = std::move(another.value_);
        return *this;
    }

    
    K key_;
    V value_;
    uint64_t expires_at_;
};

template<>
struct Entry<char*, char*> {
    ~Entry() {
        if (key_) {
            free(key_);
            key_ = nullptr;
        }
        if (value_) {
            free(value_);
            value_ = nullptr;
        }
    };
    Entry(){
        key_ = nullptr;
        value_ = nullptr;
    };
    Entry(char* key, char* value): key_(key), value_(value){};
    char* key_;
    char* value_;
    uint64_t expires_at_;
};

class TestMove{
public:

    TestMove() {
        std::cout << "zero constructor" << std::endl;
    }

    ~TestMove() {
        std::cout << "deconstructor called" << std::endl;
    }

    TestMove(double score, int a) {
        std::cout << "default constructor" << std::endl;
        score_ = score;
        a_ = a;
    }

    TestMove(TestMove&& another) {
        std::cout << "move constrcutor" << std::endl;
        score_ = another.score_;
        a_ = another.a_;
    }

    TestMove& operator=(TestMove&& str) {
        std::cout << "move assignment" << std::endl;
        score_ = str.score_;
        a_ = str.a_;
        return *this;
    }

    bool operator==(const TestMove& another) const{
        return score_ == another.score_ && a_ == another.a_;
    }


    TestMove(const TestMove& another) = delete;

    TestMove& operator=(const TestMove& str) = delete;

    // TestMove(const TestMove& another) {
    //     std::cout << "copy constructor" << std::endl;
    //     score_ = another.score_;
    //     a_ = another.a_;
    // }

    // TestMove& operator=(const TestMove& str) {
    //     std::cout << "copy assignment" << std::endl;
    //     score_ = str.score_;
    //     a_ = str.a_;
    //     return *this;
    // }

    double score_;
    int a_;
};

