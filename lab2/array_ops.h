#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

namespace lab2 {

inline long long single_threaded_xor(const std::vector<long long>& data) {
    long long result = 0;
    for (auto val : data) {
        if (val % 9 == 0) {
            result ^= val;
        }
    }
    return result;
}

inline long long mutex_threaded_xor(const std::vector<long long>& data, int num_threads) {
    long long global_result = 0;
    std::mutex mtx;
    std::vector<std::thread> threads;
    size_t chunk_size = data.size() / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            size_t start = i * chunk_size;
            size_t end = (i == num_threads - 1) ? data.size() : start + chunk_size;
            
            for (size_t j = start; j < end; ++j) {
                if (data[j] % 9 == 0) {
                    std::lock_guard<std::mutex> lock(mtx);
                    global_result ^= data[j];
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    return global_result;
}

inline long long atomic_threaded_xor(const std::vector<long long>& data, int num_threads) {
    std::atomic<long long> global_result{0};
    std::vector<std::thread> threads;
    size_t chunk_size = data.size() / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            size_t start = i * chunk_size;
            size_t end = (i == num_threads - 1) ? data.size() : start + chunk_size;
            
            for (size_t j = start; j < end; ++j) {
                if (data[j] % 9 == 0) {
                    long long current = global_result.load(std::memory_order_relaxed);
                    while (!global_result.compare_exchange_weak(
                        current, 
                        current ^ data[j],
                        std::memory_order_release, 
                        std::memory_order_relaxed)) 
                    {}
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    return global_result.load();
}

}
