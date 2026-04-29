#include "matrix.h"

#include <cmath>
#include <random>
#include <thread>
#include <algorithm>
#include <span>
#include <print>

void fill_matrix(std::vector<int> &matrix) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution dist(1, 100);

    for (int &cell : matrix) {
        cell = dist(eng);
    }    
}

int find_min_idx(int *row, int len) {
    int min_idx = 0;

    for (int j = 1; j < len; ++j) {
        if (row[j] < row[min_idx]) {
            min_idx = j;
        }
    }

    return min_idx;
}

void process_single_thread(std::vector<int> &matrix) {
    int matrix_size = std::sqrt(matrix.size());

    for (int i = 0; i < matrix_size; ++i) {
        int min_idx = i * matrix_size + find_min_idx(matrix.data() + i * matrix_size, matrix_size);

        std::swap(matrix[i * matrix_size + (matrix_size - i - 1)], matrix[min_idx]);
    }
}

void process_rows(std::vector<int> &matrix, int matrix_size, int start_row, int end_row) {
    for (int i = start_row; i < end_row; ++i) {
        int min_idx = i * matrix_size + find_min_idx(matrix.data() + i * matrix_size, matrix_size);

        std::swap(matrix[i * matrix_size + (matrix_size - i - 1)], matrix[min_idx]);
    }
}

void process_multi_thread(std::vector<int> &matrix, int num_threads) {
    int matrix_size = std::sqrt(matrix.size());
    int rows_per_thread = matrix_size / num_threads;
    int leftover_rows = matrix_size % num_threads;

    std::vector<std::thread> threads;

    int cur_row = 0;

    for (int i = 0; i < num_threads; ++i) {
        int end_row = cur_row + rows_per_thread + (i < leftover_rows ? 1 : 0);
        threads.emplace_back(process_rows, std::ref(matrix), matrix_size, cur_row, end_row);

        cur_row = end_row;
    }

    for (auto &t : threads) {
        t.join();
    }
}

void print_matrix(std::vector<int> &matrix) {
    int matrix_size = std::sqrt(matrix.size());
    std::span<int> matrix_span(matrix);

    for (int i = 0; i < matrix_size; ++i) {
        for (int j = 0; j < matrix_size; ++j) {
            std::print("{} ", matrix_span[i * matrix_size + j]);
        }
        std::println("");
    }
}
