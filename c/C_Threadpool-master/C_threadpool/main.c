#include "threadpool.h"

void do_work()
{
    printf("thread\n");
}

void main()
{
    /* 线程池初始化，其管理者线程及工作线程都会启动 */
    threadpool_t *thp = threadpool_create(10, 100, 100);
    printf("threadpool init ... ... \n");

    /* 接收到任务后添加 */
    threadpool_add_task(thp, do_work, 0);

    /* 销毁 */
    threadpool_destroy(thp);
}