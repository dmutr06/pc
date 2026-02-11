#include <print>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <random>

void fill_matrix(std::vector<int> &matrix) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution dist(1, 100);

    for (int &cell : matrix) {
        cell = dist(eng);
    }    
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::println(stderr, "Please specify matrix size");
        return 1;
    }

    const int matrix_size = atoi(argv[1]);

    std::vector<int> matrix(matrix_size * matrix_size);

    fill_matrix(matrix);

    std::println("{}", matrix);
}
