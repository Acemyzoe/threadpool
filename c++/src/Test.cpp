#include "ThreadPool.h"
#include <iostream>
#include "Tester.hpp"
#include <functional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
using namespace std;

void testThreadpool()
{
    ThreadPool pool(4);
    auto result = pool.enqueue([](int answer)
                               { return answer; },
                               42);
    std::cout << result.get() << std::endl;
}

int main()
{
    Tester tester("Test ThreadPool");
    tester.addTest(testThreadpool, "Test THREADPOOL");

    tester.runTests();
}
