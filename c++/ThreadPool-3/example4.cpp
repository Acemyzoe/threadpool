#include <iostream>
#include "ThreadPool.h"
using namespace std;

struct func
{
    int operator()(int n)
    {
        cout << n << endl
             << this_thread::get_id() << endl;
        return 11;
    }
};

int main()
{
    ThreadPool pool(4);
    future<int> result = pool.enqueue(func{}, 0);
    cout << result.get() << endl;
}