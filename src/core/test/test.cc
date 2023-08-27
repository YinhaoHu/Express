#include <bits/stdc++.h>

#include "utility/concurrency/thread_pool.h"
#include "utility/ipc/tcp_client.h"

using namespace express;

using namespace utility;
using namespace ipc;
using namespace concurrency;

using namespace std;

atomic_uint done_count;
std::mutex io_mutex;
static bool wrong_result = false;

static void Task();

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << format("usage: {} nthreads ntasks\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    uint nthreads = atoi(argv[1]);
    uint ntasks = atoi(argv[2]);

    ThreadPool *pThreadPool = new ThreadPool(nthreads);
    done_count.store(0);

    auto start_time = clock();

    for (uint i = 0; i < ntasks; ++i)
    {
        pThreadPool->Add([&]()
                         { Task(); });
    }

    while (done_count != ntasks)
        ;
    auto end_time = clock();

    delete pThreadPool;
    cout << format("result: {} ({} msec)\n", wrong_result ? "FAILURE" : "PASS",
                   (end_time - start_time) / (CLOCKS_PER_SEC / 1000)); 

    return 0;
}

static void Task()
{
    TCPClient client(InternetProtocol::kIPv4, "127.0.0.1", "32993");
    std::string request_arg("Hello, Express!");
    SentMessage request;
    request.SetCommunicationCode(0);
    request.Add(request_arg.c_str(), request_arg.size() + 1);
    client.Send(request);

    auto response = client.Receive();
    if (request_arg.compare((*response)[0].pData) != 0)
        wrong_result = true;

    done_count.fetch_add(1);
    client.Disconnect(); 
}