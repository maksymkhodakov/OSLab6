#include <iostream>
#include <vector>
#include <mutex>
#include <fstream>
#include <thread>
#include <chrono>

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
        std::lock_guard<std::mutex> lck{cout_mtx};
        for (const auto &row: matrix) {
            for (const auto &el: row) {
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
    }
}

int main(int argc, char **argv) {
    if (!(argc == 6 || (argc == 8 && std::stoi(argv[4]) == 0))) {
        std::cerr << "Usage: " << argv[0] << " n m k isRand pathA pathB maxThreads\n";
        return -1;
    }

    size_t n = std::stoul(argv[1]);
    size_t m = std::stoul(argv[2]);
    size_t k = std::stoul(argv[3]);
    size_t maxThreads = std::stoul(argv[argc - 1]); // New argument for max threads

    Matrix<int> first, second;

    if (std::stoi(argv[4]) == 1) {
        first = generate_rand_matrix(n, m);
        second = generate_rand_matrix(m, k);
    } else if (argc == 8) {
        first = read_matrix_from_file(argv[5], n, m);
        second = read_matrix_from_file(argv[6], m, k);
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    Matrix<int> res(n, std::vector<int>(k));

    size_t threadsToUse = std::min(maxThreads, n * k);

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < k; ++j) {
            if (threads.size() < threadsToUse) {
                threads.emplace_back(compute_el, std::ref(res[i][j]), std::cref(first), std::cref(second), i, j);
            } else {
                compute_el(res[i][j], first, second, i, j); // Compute directly if max threads reached
            }
        }
    }

    for (auto &thrd: threads) {
        if (thrd.joinable()) {
            thrd.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << "Computation with " << threadsToUse << " threads took " << duration.count() << " milliseconds.\n";

    print_matrix(res);

    return 0;
}
