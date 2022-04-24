### execq

**execq** 是一种基于任务的方法，它使用具有扩展功能的线程池思想来处理数据。它支持不同的任务源，并在 N 个线程上并行维护任务执行（根据硬件并发性）。

- 提供程序是，并允许以不同的方式执行任务`queues``streams`
- `queues`提供处理对象的能力，只需“将其放入队列”
- 支持串行和并发`queues`
- 随心所欲地工作：提交对象以处理返回非阻塞（与std：：async不同） std：：future，可用于在处理对象时获得结果`queue`
- 保持最佳线程数以避免过多的 CPU 线程上下文切换
- 从差异/“轮流”运行任务，避免对很晚添加的任务感到饥饿`queues``streams`
- 设计用于处理多个非阻塞任务（通常您不想在任务处理功能中休眠/等待）
- C++11 编译器

### 队列和流

execq 使用两种方法处理并发任务执行：以及`queue-based``stream-based`

*您可以自由使用多行程队列和流以及它们的任意组合！*

#### 1.1 基于队列的方法

旨在将对象处理为“推即忘”。推送到队列中的对象会在任何线程准备好处理它时立即进行处理。

ExecutionQueue 结合了线程池中的常用队列、同步机制和执行。

在内部执行队列中跟踪正在执行的任务。如果销毁，队列会将所有正在运行和挂起的任务标记为“已取消”。即使任务在执行前被取消，它也不会被丢弃，并且会在轮到时被调用，但使用“isCanceled”==true。

执行队列可以是：

- concurrent： 在多个线程上并行处理对象 *// CreateConcurrentExecutionQueue*
- serial：严格按照“一个接一个”的顺序处理对象。您可以确保没有任务同时执行 *// CreateSerialExecutionQueue*

execq 允许创建“IExecutionQueue”（串行和并发）实例来处理特定 IExecutionPool 中的对象。

IExecutionPool是一种不透明的线程池。同一个 IExecutionPool 对象通常与多个和`queues``streams`

现在，无需编写自己的队列并围绕它进行同步 - 一切都在内部完成！

```
#include <execq/execq.h>

// The function is called in parallel on the next free thread
// with the next object from the queue.
void ProcessObject(const std::atomic_bool& isCanceled, std::string&& object)
{
    if (isCanceled)
    {
        std::cout << "Queue has been canceled. Skipping object...";
        return;
    }
    
    std::cout << "Processing object: " << object << '\n';
}

int main(void)
{
    std::shared_ptr<execq::IExecutionPool> pool = execq::CreateExecutionPool();
    
    std::unique_ptr<execq::IExecutionQueue<void(std::string)>> queue = execq::CreateConcurrentExecutionQueue<void, std::string>(pool, &ProcessObject);
    
    queue->push("qwe");
    queue->push("some string");
    queue->push("");
    
    // when destroyed, queue waits until all tasks are executed
    
    return 0;
}
```

##### 独立串行队列

有时，您可能只需要队列的单线程实现即可按正确的顺序处理事物。为此，可以创建与池无关的串行队列。

```
#include <execq/execq.h>

// The function is called in parallel on the next free thread
// with the next object from the queue.
void ProcessObjectOneByOne(const std::atomic_bool& isCanceled, std::string&& object)
{
    if (isCanceled)
    {
        std::cout << "Queue has been canceled. Skipping object...";
        return;
    }
    
    std::cout << "Processing object: " << object << '\n';
}

int main(void)
{
    std::unique_ptr<execq::IExecutionQueue<void(std::string)>> queue = execq::CreateSerialExecutionQueue<void, std::string>(&ProcessObjectOneByOne);
    
    queue->push("qwe");
    queue->push("some string");
    queue->push("");
    
    // when destroyed, queue waits until all tasks are executed
    
    return 0;
}
```

#### 1.2 基于队列的方法：未来内在！

所有 ExecutionQueues 在将对象推送到其中时都返回 std：：future。未来对象绑定到推送对象，引用对象绑定到对象处理的结果。注意：返回的 std：：future 对象可以简单地丢弃。他们不会在 std：：future 析构函数中阻塞。

```
#include <execq/execq.h>

// The function is called in parallel on the next free thread
// with the next object from the queue.
size_t GetStringSize(const std::atomic_bool& isCanceled, std::string&& object)
{
    if (isCanceled)
    {
        std::cout << "Queue has been canceled. Skipping object...";
        return 0;
    }
    
    std::cout << "Processing object: " << object << '\n';
    
    return object.size();
}

int main(void)
{
    std::shared_ptr<execq::IExecutionPool> pool = execq::CreateExecutionPool();
    
    std::unique_ptr<execq::IExecutionQueue<size_t(std::string)>> queue = execq::CreateConcurrentExecutionQueue<size_t, std::string>(pool, &GetStringSize);
    
    std::future<size_t> future1 = queue->push("qwe");
    std::future<size_t> future2 = queue->push("some string");
    std::future<size_t> future3 = queue->push("hello future");
    
    const size_t totalSize = future1.get() + future2.get() + future3.get();

    return 0;
}
```

*execq 支持 std：：future，所以 ou 可以等到对象被处理完毕。*

#### 2. 基于流的方法。

旨在尽可能快地处理无数个任务，即在新线程可用时处理下一个任务。

execq 允许创建“IExecutionStream”对象，该对象将在池中的线程准备好执行下一个任务时执行代码。这种方法应被视为处理无限（或几乎无限）任务的最有效方法。

```
#include <execq/execq.h>

// The function is called each time the thread is ready to execute next task.
// It is called only if stream is started.
void ProcessNextObject(const std::atomic_bool& isCanceled)
{
    if (isCanceled)
    {
        std::cout << "Stream has been canceled. Skipping...";
        return;
    }
    
    static std::atomic_int s_someObject { 0 };
    
    const int nextObject = s_someObject++;
    
    std::cout << "Processing object: " << nextObject << '\n';
}

int main(void)
{
    std::shared_ptr<execq::IExecutionPool> pool = execq::CreateExecutionPool();
    
    std::unique_ptr<execq::IExecutionStream> stream = execq::CreateExecutionStream(pool, &ProcessNextObject);
    
    stream->start();
    
    // Only for example purposes. Usually here (if in 'main') could be RunLoop/EventLoop.
    // Wait until some objects are processed.
    sleep(5);
    
    return 0;
}
```

### 设计原则和技术细节

请考虑将单个执行池对象（跨整个应用程序）与多个队列和流一起使用。免费组合队列和流以实现您的目标。可以自由地将任务分配给队列或操作流，甚至可以从回调的内部进行操作。

#### “轮次”执行

execq以特殊的方式设计，用于处理队列和流的任务，以避免饥饿。

让我们假设一个简单的例子：有2个队列。首先，将 100 个对象推送到队列 #1。将 1 个对象推送到队列 #2 后。

现在，队列 #1 中很少有任务正在执行。但下一个执行任务将是队列 #2 中的任务，然后是队列 #1 中的任务。

#### 避免队列匮乏

有些任务可能非常耗费时间。这意味着它们将长时间阻止所有池线程的执行。这会导致饥饿：除非完成现有任务之一，否则不会执行任何其他队列任务。

为了防止这种情况，每个队列和流都具有自己的线程。此线程是某种“保险”线程，即使所有池的线程长时间处于繁忙状态，也可以执行队列/流中的任务。

### 有待完成的工作

- 将 std：:p ackaged_task 的使用替换为引用计数

### 测试

默认情况下，单元测试处于关闭状态。要启用它们，只需添加 CMake 选项 -DEXECQ_TESTING_ENABLE=ON