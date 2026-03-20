#include "thread_pool.hpp"

#include <chrono>

ThreadPool::ThreadPool(int threads_per_queue) {
    for (int i = 0; i < threads_per_queue; ++i) {
        workers.emplace_back(&ThreadPool::worker_loop, this, std::ref(queue1));
        workers.emplace_back(&ThreadPool::worker_loop, this, std::ref(queue2));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::scoped_lock lock(queue1.lock, queue2.lock);
        queue1.stop = true;
        queue2.stop = true;

        queue1.cond.notify_all();
        queue2.cond.notify_all();
    }

    for (std::thread &worker : workers) {
        if (worker.joinable()) worker.join();
    }
}

bool ThreadPool::enqueue(std::function<void()> task, int duration) {
    if (global_total_time + duration > MAX_TIME) {
        return false;
    }

    std::scoped_lock lock(queue1.lock, queue2.lock);


    SubQueue *target = (queue1.tasks.size() <= queue2.tasks.size()) ? &queue1 : &queue2;

    target->tasks.push({task, duration});
    global_total_time += duration;

    target->cond.notify_one();

    return true;
}

void ThreadPool::worker_loop(SubQueue &queue) {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queue.lock);

            queue.cond.wait(lock, [&]{
                return !queue.tasks.empty() || queue.stop;
            });

            if (queue.stop && queue.tasks.empty()) return;

            task = queue.tasks.front();
            queue.tasks.pop();

            global_total_time -= task.duration;
        }

        if (task.func) {
            std::this_thread::sleep_for(std::chrono::seconds(task.duration));
            task.func();
        }
    }
}
