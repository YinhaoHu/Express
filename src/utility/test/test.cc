/**
 * @file test.cc
 * @note Finished testing modules:
 * module               version             date
 * MessageQueue             1               2023-8-7
 * Message                  1               2023-8-7
 */
#include "utility/test/tool.h"
#include "utility/ipc/message.h"
#include "utility/ipc/message_queue.h"
#include "utility/concurrency/thread_pool.h"
#include "utility/ipc/signal.h"
#include <wait.h>

using namespace express::utility::ipc;
using namespace express::utility::concurrency;
using namespace express::utility::test;

static vector<pair<string, string>> test_option_list
{
    pair<string, string>{"msg", "test class Message"},
    pair<string, string>{"mq", "test class Message Queue"},
    pair<string, string>{"tp","test thread pool"}
};

static MessageQueue *mq;

static void test_msg();
static void test_mq();
static void test_threadpool();

int main(int argc, char *argv[])
{
    test_msg();
    test_mq();
    test_threadpool();
    return 0;
}

static void test_msg()
{
    /** TEST CASE
     * Two arguments to send and receive, then compare.
     */

    uint32_t comm_code = 32;
    string arg1("This is argument 1.");
    uint64_t arg2(32993);
    size_t arg1_size = arg1.size() + 1, arg2_size = sizeof(int64_t);
    unique_ptr<char[]> buf;

    auto sender_work = [&]()
    {
        Message msg(static_cast<uint32_t>(comm_code));

        msg.Add(arg1.c_str(), arg1_size);
        msg.Add(&arg2, arg2_size);

        test("message", "generate-sender", [&]()
             { return (
                   (msg.GetHeaderField(Message::HeaderField::KBodySize) == (arg1_size + arg2_size + 2 * sizeof(size_t))) && (msg.GetHeaderField(Message::HeaderField::kNumOfFields) == 2) && (msg.GetHeaderField(Message::HeaderField::kCommunicationCode) == comm_code)); });
        buf = msg.Rawdata();
    };

    auto receiver_work = [&]()
    {
        Message::Header hdr(*reinterpret_cast<Message::Header *>(buf.get()));
        Message msg(hdr);

        msg.SetFields(buf.get() + hdr.Size());

        test("message", "check-receiver", [&]()
             {
            auto data = msg.Body();

            return 
        (
            (hdr.communication_code) == reinterpret_cast<Message::Header *>(buf.get())->communication_code
            && (arg1.compare(static_cast<const char*>(data->at(0).pointer)) == 0)
            && (arg2 == *(uint64_t*)(data->at(1).pointer))
            && (arg1_size == data->at(0).size)
            && (arg2_size == data->at(1).size)
        ) ; });
    };

    sender_work();
    receiver_work();
}

static void test_mq()
{
    bool child_return;

    string mqname("etest");
    long msgsize = 64, maxmsg = 8;

    string arg1("Hello, there!");
    int64_t arg2 = 32;
    size_t arg1_size = arg1.size() + 1, arg2_size = sizeof(int64_t);
    uint8_t comm_code = 64;
    uint prio = 12;

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("test_mq-fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) // child.
    {
        sleep(1);
        MessageQueue cmq(mqname, maxmsg, msgsize);

        Message msg(comm_code);
        msg.Add(arg1.c_str(), arg1_size);
        msg.Add(&arg2, arg2_size);

        cmq.Send(msg, prio);
        exit(EXIT_SUCCESS);
    }
    else
    { 
        mq = new MessageQueue(mqname, maxmsg, msgsize);

        auto buf = mq->Receive();
        auto &msg = buf->message; 
        auto body = msg.Body();

        child_return =
            ((msg.GetHeaderField(Message::HeaderField::kCommunicationCode) == comm_code) && (arg1.compare(static_cast<const char *>(body->at(0).pointer)) == 0) && (arg2 == (*static_cast<const int64_t *>(body->at(1).pointer))) && (body->at(0).size == arg1_size) && (body->at(1).size == arg2_size));

        mq->Unlink();
    }
    wait(nullptr);

    test("mq", "compare", [&]()
         { return child_return == true; });
}

static void test_threadpool()
{
    ThreadPool tp(4);
    size_t ntasks = .0 ;
    vector<size_t> results;
    vector<size_t> correct;
    auto func = [](size_t input) 
    { return input + 10000; };

    for (size_t i = 0; i < ntasks; ++i)
    {
        correct.push_back(func(i));
        auto f = tp.Add(func, i);
        if (!f.valid())
            f.wait();
        results.push_back(f.get());
    }
    sort(results.begin(), results.end());

    test("thread_pool", "compare", [&](){
        for(size_t i = 0; i < results.size(); ++i)
        {
            if(results[i] != correct[i])
                return false;
        }
        return true;
    });

}

