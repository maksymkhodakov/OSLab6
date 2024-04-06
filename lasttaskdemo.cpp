#include <iostream>
#include <vector>
#include <mutex>
#include <fstream>
#include <thread>
#include <condition_variable>

namespace {
    std::mutex shared_var_mutex;
    std::condition_variable cv;
    bool turn = false; // false for t1's turn, true for t2's turn
    long long shared_var = 0;

    void safe_increment(bool myTurn) {
        for (int i = 0; i < 500; ++i) { // Each thread only needs to increment 500 times to reach 1000 together
            std::unique_lock<std::mutex> lock(shared_var_mutex);
            cv.wait(lock, [myTurn]{ return turn == myTurn; }); // Wait for my turn
            ++shared_var;
            turn = !turn; // Toggle turn
            cv.notify_one(); // Notify the other thread
        }
    }
}

int main() {
    std::thread t1(safe_increment, false); // Thread 1 starts first
    std::thread t2(safe_increment, true); // Thread 2 waits for its turn
    t1.join();
    t2.join();

    std::cout << "Final value of shared_var: " << shared_var << std::endl;

    return 0;
}
