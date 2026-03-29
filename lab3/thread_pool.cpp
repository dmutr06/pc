#include "thread_pool.hpp"

#include <chrono>

ThreadPool::ThreadPool(int threads_per_queue) {
  for (int i = 0; i < threads_per_queue; ++i) {
    workers.emplace_back(&ThreadPool::worker_loop, this, std::ref(queue1));
    workers.emplace_back(&ThreadPool::worker_loop, this, std::ref(queue2));
  }
}

ThreadPool::~ThreadPool() { shutdown(false); }

void ThreadPool::pause() {
  std::scoped_lock lock(queue1.lock, queue2.lock);
  queue1.paused = true;
  queue2.paused = true;
}

void ThreadPool::resume() {
  {
    std::scoped_lock lock(queue1.lock, queue2.lock);
    queue1.paused = false;
    queue2.paused = false;
  }
  queue1.cond.notify_all();
  queue2.cond.notify_all();
}

void ThreadPool::shutdown(bool immediate) {
  {
    std::scoped_lock lock(queue1.lock, queue2.lock);
    if (queue1.stop || queue1.stop_immediate)
      return;

    if (immediate) {
      queue1.stop_immediate = true;
      queue2.stop_immediate = true;

      while (!queue1.tasks.empty()) {
        queue1.total_duration -= queue1.tasks.front().duration;
        queue1.tasks.pop();
      }
      while (!queue2.tasks.empty()) {
        queue2.total_duration -= queue2.tasks.front().duration;
        queue2.tasks.pop();
      }
    } else {
      queue1.stop = true;
      queue2.stop = true;
    }
  }

  queue1.cond.notify_all();
  queue2.cond.notify_all();
  queue1.sleep_cond.notify_all();
  queue2.sleep_cond.notify_all();

  for (std::thread &worker : workers) {
    if (worker.joinable())
      worker.join();
  }
}

bool ThreadPool::enqueue(std::function<void()> task, int duration) {
  std::scoped_lock lock(queue1.lock, queue2.lock);

  if (queue1.stop || queue1.stop_immediate) {
    return false;
  }

  SubQueue *target =
      (queue1.tasks.size() <= queue2.tasks.size()) ? &queue1 : &queue2;

  if (target->total_duration + duration > MAX_TIME) {
    return false;
  }

  target->tasks.push({task, duration});
  target->total_duration += duration;

  target->cond.notify_one();

  return true;
}

void ThreadPool::worker_loop(SubQueue &queue) {
  while (true) {
    Task task;
    {
      std::unique_lock<std::mutex> lock(queue.lock);

      queue.cond.wait(lock, [&] {
        if (queue.stop_immediate)
          return true;
        if (queue.stop)
          return true;
        return !queue.tasks.empty() && !queue.paused;
      });

      if (queue.stop_immediate)
        return;
      if (queue.stop && queue.tasks.empty())
        return;

      task = queue.tasks.front();
      queue.tasks.pop();

      queue.total_duration -= task.duration;
    }

    if (task.func) {
      {
        std::unique_lock<std::mutex> sleep_lock(queue.lock);
        queue.sleep_cond.wait_for(sleep_lock,
                                  std::chrono::seconds(task.duration),
                                  [&] { return queue.stop_immediate; });

        if (queue.stop_immediate) {
          continue;
        }
      }

      task.func();
    }
  }
}
