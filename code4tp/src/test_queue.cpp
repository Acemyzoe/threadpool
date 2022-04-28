#include "Queue.hpp"
#include "Tester.hpp"
#include <functional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include "ThreadPool.hpp"
using namespace std;
// Test that MsgUIDs generated in two different threads simultaneously are unique
// 测试消息UID是否唯一
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

    for (auto uid1 : uids1)
    {
        for (auto uid2 : uids2)
        {
            TEST_NOT_EQUALS(uid1, uid2);
        }
    }
}

// Test that messages are received in order in 1-to-1 one-way messaging scenario
// 测试在单向消息中，消息是否按顺序接收
void testMsgOrder()
{
    const int N = 1000;
    Queue queue;

    auto sender = [](int count, Queue &q)
    {
        for (int i = 0; i < count; ++i)
            q.put(Msg(i));
    };

    auto receiver = [](int count, Queue &q)
    {
        for (int i = 0; i < count; ++i)
        {
            auto m = q.get();
            TEST_EQUALS(m->getMsgId(), i);
        }
    };

    // std::thread t1(sender, N, std::ref(queue));
    // std::thread t2(receiver, N, std::ref(queue));
    // t1.join();
    // t2.join();
    ThreadPool pool(4);
    pool.enqueue(sender, N, std::ref(queue));
    pool.enqueue(receiver, N, std::ref(queue));
}

// Test that messages are received in order in 2-to-1 one-way messaging scenario
// 测试在2-to-1单向消息中接收消息的顺序
void test2To1MsgOrder()
{
    const int N = 1000;
    Queue queue;

    auto sender = [](int msgId, int count, Queue &q)
    {
        for (int i = 0; i < count; ++i)
            q.put(DataMsg<int>(msgId, i));
    };

    auto receiver = [](int count, Queue &q)
    {
        int expectedData1 = 0;
        int expectedData2 = 0;

        for (int i = 0; i < count; ++i)
        {
            auto m = q.get();
            auto &dm = dynamic_cast<DataMsg<int> &>(*m);

            if (dm.getMsgId() == 1)
            {
                TEST_EQUALS(dm.getPayload(), expectedData1);
                ++expectedData1;
            }
            else if (dm.getMsgId() == 2)
            {
                TEST_EQUALS(dm.getPayload(), expectedData2);
                ++expectedData2;
            }
            else
            {
                TEST_FAIL("Unexpected message id");
            }
        }
    };
    ThreadPool pool(4);
    pool.enqueue(sender, 1, N, std::ref(queue));
    pool.enqueue(sender, 2, N, std::ref(queue));
    pool.enqueue(receiver, 2 * N, std::ref(queue));
}

// Test putting DataMsg through the queue
// 测试将数据消息放入队列
void testDataMsg()
{
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

// Test timeout when getting message from the queue
// 测试超时从队列中获取消息
void testReceiveTimeout()
{
    Queue q;

    // Test first with a Msg in the queue that specifying timeout for get() doesn't have an effect
    q.put(Msg(1));

    auto start = std::chrono::steady_clock::now();
    auto m = q.get(10);
    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    TEST_LESS_THAN((int)dur, 10);
    TEST_EQUALS(m->getMsgId(), 1);

    // Then test with empty queue
    start = std::chrono::steady_clock::now();
    auto m2 = q.get(10);
    end = std::chrono::steady_clock::now();
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    TEST_EQUALS((int)dur, 10);
    TEST_EQUALS(m2.get(), (Msg *)nullptr);
}

// Test tryGet() method
void testTryGet()
{
    Queue q;

    auto m1 = q.tryGet();
    TEST_EQUALS(m1.get(), (Msg *)nullptr);

    q.put(Msg(1));

    auto m2 = q.tryGet();
    TEST_NOT_EQUALS(m2.get(), (Msg *)nullptr);
}

// Test 2-to-1 request-response scenario
// 测试2-to-1请求-响应场景
void testRequestResponse()
{
    const int N = 1000;

    Queue queue;

    auto requester1 = [](int count, Queue &q)
    {
        for (int i = 0; i < count; ++i)
        {
            TEST_EQUALS(q.request(Msg(i))->getMsgId(), i + count);
        }
    };

    auto requester2 = [](int count, Queue &q)
    {
        for (int i = 0; i < count; ++i)
        {
            TEST_EQUALS(q.request(Msg(i + 2 * count))->getMsgId(), i + 3 * count);
        }
    };

    auto responder = [](int count, Queue &q)
    {
        for (int i = 0; i < 2 * count; ++i)
        {
            auto m = q.get();
            q.respondTo(m->getUniqueId(), Msg(m->getMsgId() + count));
        }
    };
    ThreadPool pool(4);
    pool.enqueue(requester1, N, std::ref(queue));
    pool.enqueue(requester2, N, std::ref(queue));
    pool.enqueue(responder, N, std::ref(queue));
}

int main()
{
    // Statically assert that messages can't be copied or moved
    static_assert(!std::is_move_constructible<Msg>::value, "Msg can't be copyable");
    static_assert(!std::is_move_assignable<Msg>::value, "Msg can't be copyable");
    static_assert(!std::is_move_constructible<DataMsg<int>>::value, "DataMsg can't be copyable");
    static_assert(!std::is_move_assignable<DataMsg<int>>::value, "DataMsg can't be copyable");

    Tester tester("Test QUEUE");
    tester.addTest(testMsgUID, "Test MsgUID generation");
    tester.addTest(testMsgOrder, "Test 1-to-1 message order");
    tester.addTest(test2To1MsgOrder, "Test 2-to-1 message order");
    tester.addTest(testDataMsg, "Test DataMsg");
    tester.addTest(testReceiveTimeout, "Test receive timeout");
    tester.addTest(testTryGet, "Test tryGet");
    tester.addTest(testRequestResponse, "Test 2-to-1 request-response");
    tester.runTests();
}
