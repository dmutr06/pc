#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
  friend class TestMetrics;

private:
  struct Task {
    std::function<void()> func;
    int duration; // from 4 to 10
  };

  struct SubQueue {
    std::queue<Task> tasks;
    std::mutex lock;
    std::condition_variable cond;
    std::condition_variable sleep_cond;
    int total_duration = 0;
    bool stop = false;
    bool stop_immediate = false;
    bool paused = false;
  };

  SubQueue queue1;
  SubQueue queue2;
  std::vector<std::thread> workers;

  const int MAX_TIME = 45;

public:
  ThreadPool(int threads_per_queue = 2);
  ~ThreadPool();

  bool enqueue(std::function<void()> task, int duration);

  void pause();
  void resume();
  void shutdown(bool immediate = false);

private:
  void worker_loop(SubQueue &queue);
};
