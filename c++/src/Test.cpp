#include "ThreadPool.h"
#include <iostream>
#include "Tester.hpp"
#include <functional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
using namespace std;

/////// 测试匿名函数 ///////
void testThreadpool1()
{
    ThreadPool pool(4);
    auto result = pool.enqueue([](int answer)
                               { return answer; },
                               42);
    auto res = result.get();
    std::cout << res << std::endl;
    TEST_EQUALS(res, 42);
}

/////// 测试普通函数 ///////
void func2()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << std::this_thread::get_id() << std::endl;
}

void testTHreadpool2()
{
    ThreadPool pool(4);
    for (int i = 0; i < 4; i++)
    {
        pool.enqueue(func2);
    }

    for (int i = 0; i < 4; i++)
    {
        std::thread th(func2);
        th.join();
    }
}

/////// 测试函数队列 ///////
void testTHreadpool3()
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
}

/////// 测试变参函数 ///////
struct func4
{
    int operator()(int n)
    {
        std::cout << n << std::endl
                  << std::this_thread::get_id() << std::endl;
        return 11;
    }
};

void testThreadpool4()
{
    ThreadPool pool(4);
    future<int> result = pool.enqueue(func4{}, 0);
    std::cout << result.get() << std::endl;
}

/////// 测试类成员函数 ///////
class A
{
public:
    static int Afun(int n = 0)
    {
        cout << n << endl
             << this_thread::get_id() << endl;
        return n;
    }

    static string Bfun(int n, string str, char c)
    {
        cout << n << endl
             << str.c_str() << endl
             << (int)c << endl
             << this_thread::get_id() << endl;
        return str;
    }
};

void testThreadpool5()
{
    ThreadPool pool(4);
    A a;
    future<int> result = pool.enqueue(a.Afun, 22);
    future<string> result2 = pool.enqueue(a.Bfun, 33, "bfun", 2233);
}

int main()
{
    Tester tester("Test ThreadPool");
    tester.addTest(testThreadpool1, "Test THREADPOOL eg.1");
    tester.addTest(testTHreadpool2, "Test THREADPOOL eg.2");
    tester.addTest(testTHreadpool3, "Test THREADPOOL eg.3");
    tester.addTest(testThreadpool4, "Test THREADPOOL eg.4");
    tester.addTest(testThreadpool5, "Test THREADPOOL eg.5");
    tester.runTests();
}
