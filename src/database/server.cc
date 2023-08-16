/**
 * @file main.cc
 * @author Hoo
 * @date 2023-8-14
 * @brief The entry of data base component.
 */
#include "database.h"
#include "public.h"
#include "handler.h"
#include "utility/macro.h"
#include "utility/ipc/unix_domain_socket.h"
#include "utility/concurrency/thread_pool.h"
#include "utility/ipc/signal.h"
#include <string>
#include <cstdlib>
#include <iostream>

using namespace express;
using namespace std;
using namespace utility::concurrency;
using namespace utility::ipc;
using namespace database;

DataBase *pDataBase;
UnixDomainSocket *pUnixDomainSocket;
static ThreadPool *pThreadPool;
// static log::Log* pLog;

static void TerminateHandler(int);
static void Init(int argc, char *argv[]);
static void Run();

int main(int argc, char *argv[])
{
    Init(argc, argv);
    Run();

    return 0;
}

/**
 * @note Explaination for `argv`
 * The respective item for each is:
 * [1] Data Base Directory Name: string
 * [2] Log Message Queue name: string
 * [3] Unix Domain Socket Descriptor: int
 * [4] Num of Threads: int
 */
static void Init(int argc, char *argv[])
{
    if (argc != 5)
    {
        // TODO [LOG]Record error information.
        fprintf(stderr, "Error in start.\n");
        exit(static_cast<int>(ExitCode::kInvalidArgument));
    }
    string db_name(argv[1]), log_name(argv[2]);
    Socket uds_fd = atoi(argv[3]);
    int n_threads = atoi(argv[4]);

    try
    {
        Signal(kTerminateSignal, TerminateHandler);
        pDataBase = new DataBase(db_name);
        pThreadPool = new ThreadPool(n_threads);
        pUnixDomainSocket = new UnixDomainSocket(uds_fd);
    }
    catch (const exception &e)
    {
        // TODO [LOG]Record error information.
        exit(static_cast<int>(ExitCode::kLoggedInitFailure));
    }
}

static void Run()
{
    try
    {
        for (;;)
        {
            auto msg = pUnixDomainSocket->ReceiveMessage();
            pThreadPool->Add(handle, std::move(msg));
        }
    }
    catch (const system_error &err)
    {
        // TODO [LOG]Record error information.
        exit(static_cast<int>(ExitCode::kRuntimeFailure));
    }
}

static void TerminateHandler(int)
{ 
    delete pDataBase;
    delete pUnixDomainSocket;
    delete pThreadPool;

    exit(0);
}