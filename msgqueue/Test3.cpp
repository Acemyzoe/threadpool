#include "Queue.hpp"
#include "Msg.hpp"
#include "Tester.hpp"
#include <functional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include "ThreadPool.h"
using namespace std;
void testMsgOrder()
{
    const int N = 1000;
    Queue queue;

    auto sender = [](int count, Queue &q)
    {
        for (int i = 0; i < count; ++i)
        {
            q.put(Msg(i));
            // std::cout << "put " << i << std::endl;
        }
    };

    auto receiver = [](int count, Queue &q)
    {
        for (int i = 0; i < count; ++i)
        {
            auto m = q.get();
            TEST_EQUALS(m->getMsgId(), i);
            std::cout << "get " << m->getMsgId() << std::endl;
        }
    };

    ThreadPool pool(4);
    pool.enqueue(sender, N, std::ref(queue));
    pool.enqueue(receiver, N, std::ref(queue));
}

int main()
{

    Tester tester("Test PolyM");
    tester.addTest(testMsgOrder, "testMsgOrder");
    tester.runTests();
}
