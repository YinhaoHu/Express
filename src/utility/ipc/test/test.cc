
#include "utility/ipc/received_boundary_message.h"
#include "utility/ipc/received_stream_message.h"
#include "utility/ipc/sent_message.h"
#include "utility/test/tool.h"
#include <bits/stdc++.h>

using namespace std;
using namespace express::utility;
using namespace ipc;
using namespace misc;
using namespace test;

static void test_msg();

int main(int argc, char *argv[])
{
    test_msg();

    return 0;
}

static void test_msg()
{
    string arg1("Hello");
    uint32_t comm_code = 32993;
    int64_t arg2{12345};
    size_t arg1_size = arg1.size() + 1, arg2_size = sizeof(arg2); 
    SentMessage sent_msg;
    sent_msg.SetCommunicationCode(comm_code);
    sent_msg.Add(arg1.c_str(), arg1_size);
    sent_msg.Add((char*)&arg2, arg2_size);

    auto test_boundary_msg = [&]()
    {
        auto ipc_data = sent_msg.Data();
        ReceivedBoundaryMessage rb_msg(std::move(ipc_data));
        string rcv_arg1((char *)rb_msg[0].pData);
        int64_t rcv_arg2 = *(int64_t *)(rb_msg[1].pData);

        test::test("msg", "boundary", [&]()
                   { return (arg1 == rcv_arg1) && (arg2 == rcv_arg2) && (rb_msg[0].size == arg1_size) && (rb_msg[1].size == arg2_size) && (rb_msg.GetHeaderField(ReceivedBoundaryMessage::Header::Field::kCommunicationCode) == comm_code); });
    };

    // TODO test stream receive msg.
    //  Then write docu for memory management and try the feeling of use.
    //  Then TCP Client and Server.
    auto test_stream_msg = [&]()
    {
        using MSG = ReceivedStreamMessage; 
        auto ipc_data = sent_msg.Data();

        std::shared_ptr<MSG::Header> spHeader(new MSG::Header);

        memcpy(spHeader->GetData(), ipc_data.get(), spHeader->Size());

        MSG msg(spHeader);
        auto body_handler = msg.GetBodyHandler();
        memcpy(body_handler.get(), ipc_data.get() + spHeader->Size(), spHeader->body_size);
        msg.ValidateBody();
        string rcv_arg1((char *)msg[0].pData);
        int64_t rcv_arg2 = *(int64_t *)(msg[1].pData);

        test::test("msg", "stream", [&]()
                   { return (arg1 == rcv_arg1) && (arg2 == rcv_arg2) && (msg[0].size == arg1_size) && (msg[1].size == arg2_size) && (msg.GetHeaderField(ReceivedBoundaryMessage::Header::Field::kCommunicationCode) == comm_code); });
    };

    test_boundary_msg();
    test_stream_msg();
}