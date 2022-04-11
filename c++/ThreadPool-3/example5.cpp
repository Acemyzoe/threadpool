#include <iostream>
#include "ThreadPool.h"
using namespace std;

class A
{ //函数必须是 static 的才能使用线程池
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

int main()
{
    ThreadPool pool(4);
    A a;
    future<int> result = pool.enqueue(a.Afun, 22);
    future<string> result2 = pool.enqueue(a.Bfun, 33, "bfun", 2233);
}