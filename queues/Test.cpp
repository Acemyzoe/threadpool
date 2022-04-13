#include <iostream>
#include <functional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "Tester.hpp"
#include "CAsyncQueue.h"
#include "CMsgQueue.h"
#include "CThreadPool.h"
using namespace std;

//////测试消息队列//////
void test_msg_queue()
{
    CMsgQueue<std::string, int, int> mq;

    mq.Push("test", 10, 20);
    mq.Push("test2", 100, 200);

    std::string val;
    int num1, num2;
    auto res = mq.Pop(val, num1, num2);
    std::cout << val << "   " << num1 << "  " << num2 << std::endl;
    mq.Pop(val, num1, num2);
    std::cout << val << "   " << num1 << "  " << num2 << std::endl;
}

void Func1(int i, const std::string &msg)
{
    std::cout << i << "-->" << msg << std::endl;
}

int Func2(int i, const std::string &msg)
{
    std::cout << i << "-->" << msg << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return i;
}

class PrintTest
{
public:
    void Func1(int i, const std::string &msg)
    {
        std::cout << i << "-->" << msg << std::endl;
    }

    void Func2(int i)
    {
        std::cout << i << "-->"
                  << "测试成员函数" << std::endl;
    }
};

//////测试线程池//////
void test_thread_pool()
{
    CThreadPool pool;
    pool.Start(2);
    // 测试lambda表达式
    pool.Commit([]
                { std::cout << "测试lambda表达式" << std::endl; });
    // 测试带参数的lambda表达式
    pool.Commit([](int val)
                { std::cout << "测试带参数的lambda表达式"
                            << "-->" << val << std::endl; },
                999);

    // 测试全局函数
    pool.Commit(Func1, 100, "测试全局函数");
    pool.Commit(Func1, 101, "测试全局函数");

    // 测试成员函数
    PrintTest p;
    pool.Commit(std::mem_fn(&PrintTest::Func1), &p, 200, "测试成员函数");

    // 测试同步获取结果
    auto res = pool.Commit(Func2, 300, "测试同步获取结果");
    auto val = res.get();
    std::cout << "获取的结果:" << val << std::endl;
}

//////测试异步消息队列//////
void test_async_queue()
{
    CAsyncQueue<std::string, bool> sqlQueue;
    sqlQueue.Start(1, [](const std::string &sql)
                   {
		std::cout << sql << std::endl;
		return true; });

    sqlQueue.Push("select * from db;");
    auto ret = sqlQueue.Push("delete from db;");
    TEST_EQUALS(ret.get(), true);
}

int main()
{
    Tester tester("Test");
    tester.addTest(test_msg_queue, "test_msg_queue");
    tester.addTest(test_thread_pool, "test_thread_pool");
    tester.addTest(test_async_queue, "test_async_queue");
    tester.runTests();
}
