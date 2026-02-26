#include "matrix.h"

#include <print>
#include <chrono>
#include <cstdlib>

constexpr int NUM_RUNS = 5;

int main(int argc, char **argv) {
    if (argc < 3) {
        std::println(stderr, "Usage: benchmark <matrix_size> <num_threads>");
        return 1;
    }

    const int size = atoi(argv[1]);
    const int threads = atoi(argv[2]);

    std::vector<int> master(size * size);
    fill_matrix(master);

    // single-threaded
    double single_ms = 0.0;
    for (int r = 0; r < NUM_RUNS; ++r) {
        auto copy = master;
        auto start = std::chrono::high_resolution_clock::now();
        process_single_thread(copy);
        auto end = std::chrono::high_resolution_clock::now();
        single_ms += std::chrono::duration<double, std::milli>(end - start).count();
    }
    single_ms /= NUM_RUNS;

    // multi-threaded
    double multi_ms = 0.0;
    for (int r = 0; r < NUM_RUNS; ++r) {
        auto copy = master;
        auto start = std::chrono::high_resolution_clock::now();
        process_multi_thread(copy, threads);
        auto end = std::chrono::high_resolution_clock::now();
        multi_ms += std::chrono::duration<double, std::milli>(end - start).count();
    }
    multi_ms /= NUM_RUNS;

    double speedup = single_ms / multi_ms;

    std::println("{:.3f} {:.3f} {:.2f}", single_ms, multi_ms, speedup);
}
