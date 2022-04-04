#include <chrono>
#include <iostream>
#include <thread>
using namespace std;

int main()
{
    unsigned int con_threads;

    con_threads = thread::hardware_concurrency();
    // 参数：该函数不接受任何参数。
    // 返回值：它返回一个非负整数，表示系统支持的并发线程数。如果该值不可计算或定义不正确，则返回0。

    cout << "Number of concurrent threads supported are:" << con_threads << endl;

    return 0;
}