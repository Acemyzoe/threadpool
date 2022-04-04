#include "threadpool.h"
#include <string>
using namespace std;
class Task
{
private:
    int total = 0;

public:
    void process();
};

//任务具体实现什么功能，由这个函数实现
void Task::process()
{
    cout << this_thread::get_id() << endl;
    this_thread::sleep_for(chrono::seconds(2));
}

void func()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << std::this_thread::get_id() << std::endl;
}

int main(void)
{
    threadPool<Task> pool(4);
    // std::string str;
    // while (1)
    // {
    //     Task *task = new Task();
    //     pool.append(task);
    //     delete task;
    // }
    for (int i = 0; i < 8; ++i)
    {
        Task *task = new Task();
        pool.enqueue(task);
        delete task;
    }

    return 0;
}