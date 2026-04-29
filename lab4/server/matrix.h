#pragma once

#include <vector>

void fill_matrix(std::vector<int> &matrix);
int find_min_idx(int *row, int len);
void process_single_thread(std::vector<int> &matrix);
void process_rows(std::vector<int> &matrix, int matrix_size, int start_row, int end_row);
void process_multi_thread(std::vector<int> &matrix, int num_threads);
void print_matrix(std::vector<int> &matrix);
