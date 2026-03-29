#include "thread_pool.hpp"
#include "thread_pool.cpp"

#include <print>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <numeric>

// Mathematical wait time calculation requires knowing total test time
auto start_time = std::chrono::steady_clock::now();

std::atomic<int> total_completed_tasks{0};
std::atomic<int> tracking_completed_duration{0};
std::atomic<int> rejected_tasks{0};

class TestMetrics {
public:
    ThreadPool& pool;
    bool running = true;
    
    // Sampling metrics
    std::vector<int> q1_sizes;
    std::vector<int> q2_sizes;
    
    // Full state metrics
    double max_full_time = 0.0;
    double min_full_time = 999999.0;
    bool currently_full = false;
    std::chrono::steady_clock::time_point full_start;

    TestMetrics(ThreadPool& p) : pool(p) {}

    void run_sampler() {
        while(running) {
            // Sampling queue lengths
            int q1_len = 0, q2_len = 0;
            {
                std::unique_lock<std::mutex> lock1(pool.queue1.lock);
                q1_len = pool.queue1.tasks.size();
            }
            {
                std::unique_lock<std::mutex> lock2(pool.queue2.lock);
                q2_len = pool.queue2.tasks.size();
            }
            q1_sizes.push_back(q1_len);
            q2_sizes.push_back(q2_len);

            // Черга вважається 'заповненою', коли залишилось місця менше ніж на найбільшу задачу (10с)
            bool is_full = false;
            {
                std::unique_lock<std::mutex> lock1(pool.queue1.lock);
                std::unique_lock<std::mutex> lock2(pool.queue2.lock);
                is_full = (pool.queue1.total_duration >= pool.MAX_TIME - 10) || (pool.queue2.total_duration >= pool.MAX_TIME - 10);
            }
            if (is_full && !currently_full) {
                currently_full = true;
                full_start = std::chrono::steady_clock::now();
            } else if (!is_full && currently_full) {
                currently_full = false;
                auto now = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double>(now - full_start).count();
                if (duration > max_full_time) max_full_time = duration;
                if (duration < min_full_time) min_full_time = duration;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void print_stats() {
        auto now = std::chrono::steady_clock::now();
        double total_test_time_s = std::chrono::duration<double>(now - start_time).count();
        int num_threads = pool.workers.size();
        
        // Mathematical Wait Time
        double total_capacity_s = num_threads * total_test_time_s;
        double total_wait_s = total_capacity_s - tracking_completed_duration.load();
        if (total_wait_s < 0) total_wait_s = 0; // Guard
        double avg_wait_time = total_wait_s / num_threads;

        double q1_avg = q1_sizes.empty() ? 0 : std::accumulate(q1_sizes.begin(), q1_sizes.end(), 0.0) / q1_sizes.size();
        double q2_avg = q2_sizes.empty() ? 0 : std::accumulate(q2_sizes.begin(), q2_sizes.end(), 0.0) / q2_sizes.size();

        double avg_exec_time = total_completed_tasks > 0 ? (double)tracking_completed_duration.load() / total_completed_tasks.load() : 0.0;

        std::println("\n============= РЕЗУЛЬТАТИ ТЕСТУВАННЯ =============");
        std::println("Час тестування: {:.3f}s", total_test_time_s);
        std::println("1. Кількість створених потоків: {}", num_threads);
        std::println("2. Середній час знаходження потоку в стані очікування: {:.3f}s", avg_wait_time);
        std::println("3. Середня довжина черги 1: {:.2f} задач", q1_avg);
        std::println("   Середня довжина черги 2: {:.2f} задач", q2_avg);
        std::println("4. Середній час виконання задач: {:.3f}s", avg_exec_time);
        
        if (min_full_time == 999999.0) {
            std::println("5. Черга жодного разу не була повністю заповнена (не досягла MAX_TIME).");
        } else {
            std::println("5. Максимальний час заповненої черги: {:.3f}s", max_full_time);
            std::println("   Мінімальний час заповненої черги: {:.3f}s", min_full_time);
        }
        std::println("6. Кількість відкинутих задач: {}", rejected_tasks.load());
        std::println("=============================================\n");
    }
};

void producer_thread(ThreadPool& pool) {
    for (int i = 0; i < 30; ++i) { // Many tasks to hit MAX_TIME bound
        int duration = (i % 7) + 4; // 4 to 10 seconds
        
        bool success = pool.enqueue([duration] {
            tracking_completed_duration += duration;
            total_completed_tasks++;
        }, duration);

        if (!success) {
            rejected_tasks++;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Ultra rapid insertion
    }
}

int main() {
    std::println("Launching bounded test metrics...");
    ThreadPool pool(2); // 4 worker threads
    
    TestMetrics metrics(pool);
    std::thread sampler([&metrics] { metrics.run_sampler(); });

    std::vector<std::thread> producers;
    for(int i=0; i<4; ++i) {
        producers.emplace_back(producer_thread, std::ref(pool));
    }

    std::this_thread::sleep_for(std::chrono::seconds(25));
    
    for (auto& p : producers) p.join();

    pool.shutdown(false); // Graceful shutdown

    metrics.running = false;
    sampler.join();

    metrics.print_stats();

    return 0;
}
