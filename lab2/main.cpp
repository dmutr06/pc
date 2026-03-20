#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <print>
#include <fstream>
#include <thread>
#include "array_ops.h"

int main() {
    int max_threads = std::thread::hardware_concurrency();
    if (max_threads == 0) max_threads = 4; // fallback

    std::vector<size_t> data_sizes = {1'000'000, 10'000'000, 50'000'000, 100'000'000};

    std::ofstream csv_file("benchmark_results.csv");
    csv_file << "Elements,Method,TimeMs,Result\n";

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<long long> dist(1, 1'000'000);

    for (size_t size : data_sizes) {
        std::println("Generating array of size {}...", size);
        std::vector<long long> data(size);
        for (size_t i = 0; i < size; ++i) {
            data[i] = dist(rng);
            if (i % 20 == 0) {
                data[i] = (dist(rng) % 1000) * 9;
            }
        }

        long long expected_result = -1;

        for (std::string method : {"Single", "Mutex", "Atomic"}) {
            int threads = (method == "Single") ? 1 : max_threads;

            auto start = std::chrono::high_resolution_clock::now();
            long long result = 0;

            if (method == "Single") {
                result = lab2::single_threaded_xor(data);
                expected_result = result;
            } else if (method == "Mutex") {
                result = lab2::mutex_threaded_xor(data, threads);
            } else if (method == "Atomic") {
                result = lab2::atomic_threaded_xor(data, threads);
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            if (result != expected_result && result != 0 && expected_result != -1) {
                std::println(stderr, "ERROR: Result mismatch for {} {} threads! expected {} got {}", method, threads, expected_result, result);
            }

            csv_file << size << "," << method << "," << duration << "," << result << "\n";
            std::println("Size: {:>9} | Method: {:>6} | Time: {:>7} ms | Res: {}", size, method, duration, result);
        }
    }

    csv_file.close();
    std::println("Benchmark complete. Results saved to benchmark_results.csv");
    return 0;
}
