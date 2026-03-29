#include "thread_pool.hpp"
#include "thread_pool.cpp"

#include <print>
#include <thread>
#include <vector>
#include <chrono>

auto start_time = std::chrono::steady_clock::now();

double get_time() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - start_time).count();
}

void producer_thread(ThreadPool& pool, int producer_id) {
    for (int i = 1; i <= 3; ++i) {
        int duration = (producer_id + i) % 7 + 4; // 4 to 10 seconds
        
        std::println("[{:.3f}s] [Producer {}] Enqueueing task {} (duration {}s)", get_time(), producer_id, i, duration);
        
        bool success = pool.enqueue([producer_id, i, duration] {
            std::println("[{:.3f}s] [Worker {:x}] Finished Task {} from Producer {} (took {}s in background)", 
                get_time(), std::hash<std::thread::id>{}(std::this_thread::get_id()) % 0xFFFF, i, producer_id, duration);
        }, duration);

        if (!success) {
            std::println("[{:.3f}s] [Producer {}] Failed to add task {} (pool limit or stop)", get_time(), producer_id, i);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::println("[0.000s] Main thread [{:x}] starting program...", std::hash<std::thread::id>{}(std::this_thread::get_id()) % 0xFFFF);
    ThreadPool pool(2); // 4 worker threads

    std::vector<std::thread> producers;
    for (int i = 1; i <= 3; ++i) {
        producers.emplace_back(producer_thread, std::ref(pool), i);
    }

    for (auto& prod : producers) {
        prod.join();
    }
    
    std::println("[{:.3f}s] Main thread waiting for pool to gracefully finish remaining tasks...", get_time());
    pool.shutdown(false);
    
    std::println("[{:.3f}s] Main thread finished.", get_time());
    return 0;
}
