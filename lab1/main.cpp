#include "matrix.h"

#include <print>
#include <cstdlib>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::println(stderr, "Please specify matrix size");
        return 1;
    }

    const int matrix_size = atoi(argv[1]);

    std::vector<int> matrix(matrix_size * matrix_size);

    fill_matrix(matrix);
    print_matrix(matrix);
    process_multi_thread(matrix, 2);
    std::println("-------");
    print_matrix(matrix);
}
