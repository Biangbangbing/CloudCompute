#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <unistd.h>

using namespace std;

class ThreadPool {
public:
    int size;

    using TaskEntry = function<void()>;

    ThreadPool(int size0) {
        running.store(false);
        stopflag.store(false);
        int enableCpuNums = thread::hardware_concurrency();
        size = (size0 > 0 && size0 < enableCpuNums) ? size0 : enableCpuNums;
    }

    ~ThreadPool() {
        if (running.load()) {
            stop();
        }
    }

    void start() {
        running.store(true);
        threadBucket.reserve(size);
        for (int i = 0; i < size; i++) {
            threadBucket.emplace_back(new thread(bind(&ThreadPool::TaskInitEntry, this)));
        }
    }

    void stop() {
        stopflag.store(true);
        while (1) {
            threadPoolMutex.lock();
            int n = taskbucket.size();
            threadPoolMutex.unlock();
            if (n == 0) break;
        }
        running.store(false);
        //notify_one()与notify_all()常用来唤醒阻塞的线程
            convar.notify_all();
        for (int i = 0; i < size; i++) {
            threadBucket[i]->join();
        }
    }

    void run(TaskEntry task) {
        if (stopflag.load() || !running.load() || !task) return;
        unique_lock<mutex> lock(threadPoolMutex);
        taskbucket.push_back(task);
        convar.notify_one();
    }

    int getSize() {
        return size;
    }

private:
    void TaskInitEntry() {
        while (running.load()) {
            unique_lock<mutex>lock(threadPoolMutex);
            convar.wait(lock, [this] {return !running.load() || !taskbucket.empty(); });
            if (!running.load() && taskbucket.empty()) return;
            TaskEntry task = taskbucket.front();
            taskbucket.pop_front();
            task();
        }
    }

private:
    //atomic是原子变量
    atomic<bool>running;
    atomic<bool>stopflag;
    mutex threadPoolMutex;
    condition_variable convar;
    //unique_ptr是智能指针，指向一个Thread类型，Vector对其进行存储。
    vector<unique_ptr<thread>> threadBucket;
    //deque是双向队列，存储的是任务列表
    deque<TaskEntry>taskbucket;
};

