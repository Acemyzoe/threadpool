#include <iostream>
#include "threadpool.h"
// #include "threadpool2.h"

void func()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << std::this_thread::get_id() << std::endl;
}

int main()
{
    ThreadPool pool(4);
    // while (1)
    // {
    //     pool.enqueue(func);
    // }
    for (int i = 0; i < 4; i++)
    {
        pool.enqueue(func);
    }

    pool.enqueue([]()
                 { std::cout << "lambda" << std::endl; });

    for (int i = 0; i < 4; i++)
    {
        std::thread th(func);
        th.join();
    }
}