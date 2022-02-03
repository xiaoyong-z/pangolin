#ifndef THREAD_H
#define THREAD_H

#include <memory>
#include <thread>

class Runnable {
public:
    virtual void run() = 0;
    virtual ~Runnable() = default;
    void start() {
        flag_.store(true);
    }

    void stop() {
        assert (flag_.load() == true);
        std::cout << "stop" << std::endl;
        flag_.store(false);
    }

    bool isRunning() {
        bool flag = flag_.load();
        return flag; 
    }
private:
    std::atomic<bool> flag_;
};

class Thread {
public:
    Thread(): stopped_(true) {};

    ~Thread() {
        if (stopped_ == false) {
            run_class_->stop();
            t->join();
            delete run_class_;
        }
    }

    void start(Runnable* run_class) noexcept {
        stopped_ = false;
        run_class_ = run_class;
        run_class_->start();
        t = std::make_unique<std::thread>(&Runnable::run, run_class_);
    }

    void stop() {
        run_class_->stop();
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