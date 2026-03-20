#include "thread_pool.hpp"

#include <cstdio>

#include "thread_pool.cpp"

int main() {
    ThreadPool thread_pool;

    thread_pool.enqueue([]{ printf("Hello, World1!\n"); }, 44);
    thread_pool.enqueue([]{ printf("Hello, World2!\n"); }, 1);
    thread_pool.enqueue([]{ printf("Hello, World3!\n"); }, 5);
    thread_pool.enqueue([]{ printf("Hello, World4!\n"); }, 5);
    thread_pool.enqueue([]{ printf("Hello, World5!\n"); }, 5);
    thread_pool.enqueue([]{ printf("Hello, World6!\n"); }, 5);

    return 0;
}
