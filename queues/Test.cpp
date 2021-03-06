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
    void CFunc1(int i, const std::string &msg)
    {
        std::cout << i << "-->" << msg << std::endl;
    }

    void CFunc2(int i)
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
    pool.Commit(std::mem_fn(&PrintTest::CFunc1), &p, 200, "测试成员函数");
    pool.Commit(std::mem_fn(&PrintTest::CFunc2), &p, 201);

    // 测试同步获取结果
    auto res = pool.Commit(Func2, 300, "测试同步获取结果");
    auto val = res.get();
    std::cout << "获取的结果:" << val << std::endl;
    auto res2 = pool.Commit([](int val)
                            { return val + 1; },
                            val);
    std::cout << "获取的结果:" << res2.get() << std::endl;
}

//////测试异步队列//////
void test_async_queue()
{
    CAsyncQueue<std::string, bool> sqlQueue;
    sqlQueue.Start(4, [](const std::string &sql)
                   {
		std::cout << sql << std::endl;
        std::cout << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
		return true; });

    sqlQueue.Push("select * from db;");
    auto ret = sqlQueue.Push("delete from db;");
    TEST_EQUALS(ret.get(), true);

    sqlQueue.Push("select * from db2;");
    auto ret2 = sqlQueue.Push("delete from db2;");
    TEST_EQUALS(ret2.get(), true);
}

//  测试线程池+消息队列。
#include "ThreadPool.h"
void testMsgOrder()
{
    CMsgQueue<std::string, int, int> mq;
    ThreadPool pool(2);
    int count = 100;
    pool.enqueue([&mq](int count)
                 {   std::cout<< this_thread::get_id() << std::endl; 
                     for (int i = 0; i < count; i++)
                        {
                            mq.Push("test", i, i);
                            // std::cout << i << std::endl;
                        } },
                 count);

    pool.enqueue([&mq, &pool](int count)
                 {std::string val;
                int num1, num2;
                std::cout<< this_thread::get_id() << std::endl; 
                std::cout << pool.idlCount() << "vs"<< pool.thrCount() << std::endl;

                for (int i = 0; i < count; i++)
                {
                    auto res = mq.Pop(val, num1, num2);
                    TEST_EQUALS(val, std::string("test"));
                    TEST_EQUALS(num1, i);
                    TEST_EQUALS(num2, i);
                } },
                 count);
}

int main()
{
    Tester tester("Test");
    // tester.addTest(test_msg_queue, "test_msg_queue");
    // tester.addTest(test_thread_pool, "test_thread_pool");
    // tester.addTest(test_async_queue, "test_async_queue");
    tester.addTest(testMsgOrder, "testMsgOrder");
    tester.runTests();
}
