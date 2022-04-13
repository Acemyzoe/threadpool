#include "CThreadPool.h"
#include "CMsgQueue.h"

#include <iostream>
#include "Tester.hpp"
#include <functional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
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

int main()
{
    Tester tester("Test");
    tester.addTest(test_msg_queue, "test_msg_queue");
    tester.runTests();
}
