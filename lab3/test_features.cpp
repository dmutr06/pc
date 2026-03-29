#include "thread_pool.hpp"
#include "thread_pool.cpp"
#include <print>
#include <chrono>

int main() {
    ThreadPool pool(2); // 4 workers
    
    // Normal queue
    pool.enqueue([]{ std::println("Task 1 completed"); }, 1);
    pool.enqueue([]{ std::println("Task 2 completed"); }, 1);
    pool.enqueue([]{ std::println("Task 3 completed"); }, 1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    std::println("--- Pause test ---");
    pool.pause();
    
    // These should not execute while paused
    pool.enqueue([]{ std::println("Task 4 completed (should be after resume)"); }, 1);
    pool.enqueue([]{ std::println("Task 5 completed (should be after resume)"); }, 1);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::println("--- Resume test ---");
    pool.resume();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::println("--- Immediate shutdown test ---");
    // Long task
    pool.enqueue([]{ std::println("Task 6 completed (Long task!)"); }, 10);
    // Task that should be discarded
    pool.enqueue([]{ std::println("Task 7 completed (SHOULD NEVER PRINT!)"); }, 2);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    pool.shutdown(true); // stop immediately
    
    std::println("Test finished.");
    return 0;
}
