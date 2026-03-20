#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>

class ThreadPool {
private:
    struct Task {
        std::function<void()> func;
        int duration; // from 4 to 10
    };

    struct SubQueue {
        std::queue<Task> tasks;
        std::mutex lock;
        std::condition_variable cond;
        bool stop = false;
    };

    SubQueue queue1;
    SubQueue queue2;
    std::vector<std::thread> workers;
    std::atomic<int> global_total_time{0};

    const int MAX_TIME = 45;

public:
    ThreadPool(int threads_per_queue = 2);
    ~ThreadPool();

    bool enqueue(std::function<void()> task, int duration);

private:
    void worker_loop(SubQueue &queue);
};
