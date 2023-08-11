#include "utility/ipc/message_queue.h"
#include "utility/ipc/received_boundary_message.h"
#include "utility/ipc/received_stream_message.h"
#include "utility/ipc/tcp_client.h"
#include "utility/ipc/tcp_server.h"
#include "utility/ipc/sent_message.h"
#include "utility/test/tool.h"
#include <bits/stdc++.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/mman.h>

using namespace std;
using namespace express::utility;
using namespace ipc;
using namespace misc;
using namespace test;

static void test_msg();
static void test_mq();
static void test_tcp();


int main(int argc, char *argv[])
{
    test_msg();
    test_mq();
    test_tcp();

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
    sent_msg.Add((char *)&arg2, arg2_size);

    auto test_boundary_msg = [&]()
    {
        auto ipc_data = sent_msg.Data();
        ReceivedBoundaryMessage rb_msg(std::move(ipc_data));
        string rcv_arg1((char *)rb_msg[0].pData);
        int64_t rcv_arg2 = *(int64_t *)(rb_msg[1].pData);

        test::test("msg", "boundary", [&]()
                   { return (arg1 == rcv_arg1) && (arg2 == rcv_arg2) && (rb_msg[0].size == arg1_size) && (rb_msg[1].size == arg2_size) && (rb_msg.GetHeaderField(ReceivedBoundaryMessage::Header::Field::kCommunicationCode) == comm_code); });
    };

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

static void test_mq()
{
    MessageQueue *pMq;
    bool compare_correct;

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
        MessageQueue cmq(mqname, maxmsg, msgsize);

        SentMessage msg;

        msg.SetCommunicationCode(comm_code);
        msg.Add(arg1.c_str(), arg1_size);
        msg.Add((char *)&arg2, arg2_size);
        cmq.Send(msg, prio);
        exit(EXIT_SUCCESS);
    }
    else
    {
        pMq = new MessageQueue(mqname, maxmsg, msgsize);

        auto data = pMq->Receive();
        ReceivedBoundaryMessage msg(data->message);

        string recv_arg1(msg[0].pData);
        size_t recv_arg1_size = msg[0].size;
        bool arg1_correct = (recv_arg1 == arg1 && arg1_size == recv_arg1_size);

        int64_t recv_arg2 = *(int64_t *)(msg[1].pData);
        size_t recv_arg2_size = msg[1].size;
        bool arg2_correct = (recv_arg2 == arg2 && recv_arg2_size == arg2_size);

        bool comm_code_correct = (msg.GetHeaderField(ReceivedBoundaryMessage::Header::Field::kCommunicationCode) == comm_code);

        compare_correct = (arg1_correct && arg2_correct && comm_code_correct);

        pMq->Unlink();
    }
    wait(nullptr);

    test::test("mq", "compare", [&]()
               { return compare_correct == true; });
}


static void test_tcp()
{  
    const char* ip_address = "127.0.0.1", *port = "32993";

    // Synchronization preparation
    const string shm_name("/test_tcp");
    size_t sm_area_size = sizeof(uint64_t);
    int sm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(sm_fd < 0)
    {
        perror("test_tcp, shared memory");
        exit(EXIT_FAILURE);
    }
    ftruncate(sm_fd, sm_area_size);   

    const string comm_data("Hello,Express!");
    const size_t comm_data_size = comm_data.size() +1;
    const uint32_t comm_code = 1234;

    const size_t num_of_test = 6;

    int pid = fork();
    if(pid < 0)
    {
        perror("test_tcp-fork");
        exit(EXIT_FAILURE);
    }
    else if(pid  == 0)  // Child.
    {    
        uint64_t* ptr = (uint64_t*)mmap(nullptr, sm_area_size, PROT_READ| PROT_WRITE,
            MAP_SHARED, sm_fd, 0);

        while(*ptr != 1);

        for(size_t count = 0; count < num_of_test; ++count )
        { 
            TCPClient client(InternetProtocol::kIPv4, ip_address, port);

            SentMessage msg;
            msg.SetCommunicationCode(comm_code);
            msg.Add(comm_data.c_str(), comm_data_size);
            client.Send(msg);  
        } 

        exit(EXIT_SUCCESS);
    }
    else //Parent
    {
        uint64_t* ptr = (uint64_t*)mmap(nullptr, sm_area_size, PROT_READ| PROT_WRITE,
            MAP_SHARED, sm_fd, 0);
        
        using Message = ReceivedStreamMessage;

        size_t test_count = 0;
        TCPServer server(port, InternetProtocol::kIPv4);  
        *ptr = 1; // Tell child process to start.
        while(test_count < num_of_test)
        {
            if(!server.HasPendingConnections())
                continue;
            auto client = server.NextPending();

            shared_ptr<Message::Header> spHeader(new Message::Header);
            client->Receive(spHeader->GetData(), spHeader->Size(), MSG_WAITALL);
            
            Message msg(spHeader);
            client->Receive(msg.GetBodyHandler().get(), spHeader->body_size, MSG_WAITALL);
            msg.ValidateBody();

            string recv_data{msg[0].pData};

            test::test("tcp", "compare", [&]()
            {
                bool comm_data_correct = (recv_data == comm_data && msg[0].size == comm_data_size);
                bool comm_meta_correct = 
                    (comm_code && msg.GetHeaderField(Message::Header::Field::kCommunicationCode));

                return comm_data_correct && comm_meta_correct;
            });
            ++test_count; 
        }
        server.Close(); 

        wait(nullptr);
        shm_unlink(shm_name.c_str());
        exit(EXIT_FAILURE);
    }
}