#include "CMsgQueue.h"
int main()
{
    CMsgQueue<std::string, int, int> mq;

    mq.Push("test", 10, 20);
    mq.Push("test2", 100, 200);

    std::string val;
    int num1, num2;
    mq.Pop(val, num1, num2);
    std::cout << val << "   " << num1 << "  " << num2 << std::endl;
    mq.Pop(val, num1, num2);
    std::cout << val << "   " << num1 << "  " << num2 << std::endl;
    return 0;
}