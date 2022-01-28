#include <iostream>
#include "gtest/gtest.h"
#include "thread.h"

class A: public Runnable {
public:
    A(): a(0){}

    ~A() {
        std::cout << "release called" << std::endl;
    }

    void run() override{
        a++;
        std::cout << a << std::endl;
    }
private:
    int a;
};

TEST(ThreadTest, basic_test) {
    std::vector<std::unique_ptr<Thread>> threads;

    for (int i = 0; i < 5; i++) {
        threads.emplace_back(std::make_unique<Thread>());
        A* a = new A();
        threads[i]->start(a);
    }

    // for (int i = 0; i < 5; i++) {
    //     threads[i]->stop();
    // }
    
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
