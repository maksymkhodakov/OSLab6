#include <iostream>
#include <vector>
#include <mutex>
#include <fstream>
#include <thread>


namespace {
    constexpr size_t kMaxElement{100};
    template<typename T>
    using Matrix = std::vector<std::vector<T>>;

    std::mutex cout_mtx{};

    Matrix<int> generate_rand_matrix(size_t rows, size_t columns) {
        Matrix<int> res(rows, std::vector<int>(columns));
        for (auto &row: res) {
            for (auto &el: row) {
                el = std::rand() % kMaxElement;
            }
        }
        return res;
    }

    void print_matrix(const Matrix<int> &matrix) {
        for (const auto &row: matrix) {
            for (const auto &el: row) {
                std::lock_guard lck{cout_mtx};
                std::cout << el << " ";
            }
            std::cout << "\n";
        }
    }

    Matrix<int> read_matrix_from_file(const std::string &path, size_t rows, size_t columns) {
        std::ifstream fin(path);
        Matrix<int> res(rows, std::vector<int>(columns));
        int tmp{};
        for (auto &row: res) {
            for (auto &el: row) {
                fin >> tmp;
                el = tmp;
            }
        }
        return res;
    }

    void compute_el(int &res, const Matrix<int> &a, const Matrix<int> &b, size_t i, size_t j) {
        if (a[i].size() != b.size()) {
            exit(-2);
        }
        res = 0;
        size_t m = a[i].size();
        for (size_t cntr = 0; cntr < m; ++cntr) {
            res += a[i][cntr] * b[cntr][j];
        }

        {
            std::lock_guard lck{cout_mtx};
            std::cout << "[" << i << j << "] computed\n";
        }
    }

}


// ./a.out n m k isRand pathA pathB
int main(int argc, char **argv) {

    if (!(argc == 5 || (argc == 7 && std::stoi(argv[4]) == 0))) {
        return -1;
    }
    Matrix<int> first;
    Matrix<int> second;

    size_t n = std::stoul(argv[1]);
    size_t m = std::stoul(argv[2]);
    size_t k = std::stoul(argv[3]);

    if (argc == 5) {
        first = generate_rand_matrix(n, m);
        second = generate_rand_matrix(m, k);
    }

    if (argc == 7) {
        first = read_matrix_from_file(argv[5], n, m);
        second = read_matrix_from_file(argv[6], m, k);
    }


    print_matrix(first);
    print_matrix(second);

    std::vector<std::thread> threads;

    Matrix<int> res(n, std::vector<int>(k));

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < k; ++j) {
            threads.emplace_back(compute_el, std::ref(res[i][j]), first, second, i, j);
        }
    }

    for (auto &thrd: threads) {
        thrd.join();
    }

    print_matrix(res);

    return 0;
}
