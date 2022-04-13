#include "CThreadPool.h"
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
    CThreadPool pool;
    pool.Start(4);
    auto result = pool.Commit([](int answer)
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
    CThreadPool pool;
    pool.Start(4);
    for (int i = 0; i < 4; i++)
    {
        pool.Commit(func2);
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
    CThreadPool pool;
    pool.Start(4);
    std::vector<std::future<int>> results;
    for (int i = 0; i < 5; ++i)
    {
        results.emplace_back(
            pool.Commit([i]
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
    CThreadPool pool;
    pool.Start(4);
    future<int> result = pool.Commit(func4{}, 0);
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
    CThreadPool pool;
    pool.Start(4);
    A a;
    future<int> result = pool.Commit(a.Afun, 22);
    future<string> result2 = pool.Commit(a.Bfun, 33, "bfun", 2233);
    cout << result.get() << result2.get() << endl;
}

int main()
{
    Tester tester("Test CThreadPool");
    tester.addTest(testThreadpool1, "Test THREADPOOL eg.1");
    tester.addTest(testTHreadpool2, "Test THREADPOOL eg.2");
    tester.addTest(testTHreadpool3, "Test THREADPOOL eg.3");
    tester.addTest(testThreadpool4, "Test THREADPOOL eg.4");
    tester.addTest(testThreadpool5, "Test THREADPOOL eg.5");
    tester.runTests();
}
