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
// Test that MsgUIDs generated in two different threads simultaneously are unique
void testMsgUID()
{
    const int N = 1000;
    std::vector<MsgUID> uids1, uids2;

    auto createMsgs = [](int count, std::vector<MsgUID> &uids)
    {
        for (int i = 0; i < count; ++i)
            uids.push_back(Msg(1).getUniqueId());
    };

    // std::thread t1(createMsgs, N, std::ref(uids1));
    // std::thread t2(createMsgs, N, std::ref(uids2));
    // t1.join();
    // t2.join();
    ThreadPool pool(4);
    pool.enqueue(createMsgs, N, std::ref(uids1));
    pool.enqueue(createMsgs, N, std::ref(uids2));

    Queue q;
    q.put(DataMsg<std::string>(42, "foo"));
    auto m = q.get();
    auto &dm = dynamic_cast<DataMsg<std::string> &>(*m);
    TEST_EQUALS(dm.getMsgId(), 42);
    TEST_EQUALS(dm.getPayload(), std::string("foo"));
    // Test modifying the payload data
    dm.getPayload() += "bar";
    TEST_EQUALS(dm.getPayload(), std::string("foobar"));
}

int main()
{

    Tester tester("Test PolyM");
    tester.addTest(testMsgUID, "Test MsgUID generation");
    tester.runTests();
}
