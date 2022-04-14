#include "CThreadPool.h"
#include <iostream>
using namespace std;

void test_thread_pool()
{
    CThreadPool<int> pool(4, [](int &record, CThreadPool<int> *pSub)
                          {
        cout << "record: " << record << endl;
    return true; });

    pool.Run();
    for (int i = 0; i < 3; i++)
    {
        pool.Submit(i);
    }
}

int main()
{
    test_thread_pool();
    return 0;
}