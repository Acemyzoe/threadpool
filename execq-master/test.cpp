#include <execq/execq.h>
#include <iostream>
// The function is called each time the thread is ready to execute next task.
// It is called only if stream is started.
void ProcessNextObject(const std::atomic_bool &isCanceled)
{
    if (isCanceled)
    {
        std::cout << "Stream has been canceled. Skipping...";
        return;
    }

    static std::atomic_int s_someObject{0};

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
    std::cin.get();

    return 0;
}