#include "thread_pool.hpp"
#include "thread_pool.cpp"
#include <print>
#include <chrono>

int main() {
    ThreadPool pool(1); // 2 workers (1 per queue)
    
    // Fill up the workers with long tasks
    pool.enqueue([]{ std::println("Worker 1 task"); }, 3);
    pool.enqueue([]{ std::println("Worker 2 task"); }, 3);
    
    // These will be in the queue
    for(int i=0; i<10; ++i) {
        pool.enqueue([i]{ std::println("Task {} completed (SHOULD BE DROPPED!)", i+3); }, 5);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::println("Calling immediate shutdown...");
    pool.shutdown(true); // stop immediately, dropping queued tasks
    
    std::println("Test finished. It should exit soon and NOT print 'SHOULD BE DROPPED'.");
    return 0;
}
