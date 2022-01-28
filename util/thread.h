#ifndef THREAD_H
#define THREAD_H

#include <memory>
#include <thread>

class Runnable {
public:
    virtual void run() = 0;
    virtual ~Runnable() = default;
};

class Thread {
public:
    Thread(): stopped_(true) {};

    ~Thread() {
        if (stopped_ == false) {
            t->join();
            delete run_class_;
        }
    }

    void start(Runnable* run_class) noexcept {
        stopped_ = false;
        run_class_ = run_class;
        t = std::make_unique<std::thread>(&Runnable::run, run_class_);
    }

    void stop() {
        stopped_ = true;
        t->join();
        delete run_class_;
    }

private:
    bool stopped_;
    std::unique_ptr<std::thread> t;
    Runnable* run_class_;
};

#endif