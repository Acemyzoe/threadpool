#include <iostream>
#include <vector>
#include <chrono>
#include "ThreadPool.h"
using namespace std;
int main()
{

    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 5; ++i)
    {
        results.emplace_back(
            pool.enqueue([i]
                         {
                std::cout << std::this_thread::get_id() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                return i; }));
    }

    for (auto &&result : results)
        std::cout << result.get() << std::endl;

    return 0;
}
// g++ example3.cpp -o example3 -pthread