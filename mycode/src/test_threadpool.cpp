#include "ThreadPool.hpp"
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
    TEST_EQUALS(res, 42);
}

auto func2()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // std::cout << std::this_thread::get_id() << std::endl;
    auto id = std::this_thread::get_id();
    return id;
}
/////// 测试普通函数&耗时///////
void testThreadpool2()
{
    int count = 4;
    //线程池
    auto start = std::chrono::steady_clock::now();
    ThreadPool pool(4);
    std::vector<std::future<std::thread::id>> results;

    for (int i = 0; i < count; i++)
    {
        results.emplace_back(pool.enqueue(func2));
    }

    for (auto &&result : results)
        std::cout << result.get() << std::endl;

    auto end = std::chrono::steady_clock::now();
    auto dur1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "线程池耗时：" << dur1 << "ms" << std::endl;

    //单线程,异步任务
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < count; i++)
    {
        // std::thread th(func2);
        // th.join();
        std::future<std::thread::id> res = std::async(std::launch::async, func2);
        auto id = res.get();
        cout << id << endl;
    }
    end = std::chrono::steady_clock::now();
    auto dur2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "单线程耗时：" << dur2 << "ms" << std::endl;
    TEST_NOT_EQUALS(dur1, dur2);
}

/////// 测试函数队列 ///////
void testThreadpool3()
{
    ThreadPool pool(4);
    std::vector<std::future<int>> results;
    for (int i = 0; i < 5; ++i)
    {
        results.emplace_back(
            pool.enqueue([i]
                         {
            // auto id = std::this_thread::get_id();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return i; }));
    }
    for (auto &&result : results)
        std::cout << result.get() << std::endl;
}

struct func4
{
    int operator()(int n)
    {
        // std::cout << n << std::endl
        //           << std::this_thread::get_id() << std::endl;
        return n;
    }
};
/////// 测试变参函数 ///////
void testThreadpool4()
{
    ThreadPool pool(4);
    future<int> result = pool.enqueue(func4{}, 233);
    // std::cout << result.get() << std::endl;
    TEST_EQUALS(result.get(), 233);
}

class A
{
public:
    static int Afun(int n = 0)
    {
        // cout << n << this_thread::get_id() << endl;
        return n;
    }

    static string Bfun(int n, string str)
    {
        return std::to_string(n) + str;
    }
};

/////// 测试类成员函数 ///////
void testThreadpool5()
{
    ThreadPool pool(4);
    // A a;
    // future<int> result = pool.enqueue(a.Afun, 22);
    // future<string> result2 = pool.enqueue(a.Bfun, 33, "bfun");
    // cout << result.get() << result2.get() << endl;

    auto res = pool.enqueue(A::Afun, 22);
    auto res2 = pool.enqueue(A::Bfun, 33, "bfun");
    TEST_EQUALS(res.get(), 22);
    TEST_EQUALS(res2.get(), (string) "33bfun");
}

int main()
{
    Tester tester("Test ThreadPool");
    tester.addTest(testThreadpool1, "Test THREADPOOL eg.1");
    tester.addTest(testThreadpool2, "Test THREADPOOL eg.2");
    tester.addTest(testThreadpool3, "Test THREADPOOL eg.3");
    tester.addTest(testThreadpool4, "Test THREADPOOL eg.4");
    tester.addTest(testThreadpool5, "Test THREADPOOL eg.5");
    tester.runTests();
}
