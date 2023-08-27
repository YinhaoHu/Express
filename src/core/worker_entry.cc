#include "reactor.h"
#include "common.h"
#include "utility/ipc/socket.h"
#include "utility/ipc/unix_domain_socket.h"
#include "utility/ipc/signal.h"

using express::core::Reactor;
using express::utility::ipc::Signal;
using express::utility::ipc::UnixDomainSocket;

static Reactor *pReactor;
static UnixDomainSocket *pMasterChannel;

static void RunController(int master_channel, int heart_beat_rate);
static void HeartBeat();
static void TerminateHandler(int);

/**
 * @note Explaination for `argv`
 * [1]  channel_fd              ->  int     :   Unix domain socket connected to master.
 * [2]  heart_beat_rate         ->  int     :   Specify the heart beat every # sec.
 * [3]  db_channel              ->  int     :   UDS Channel to database.
 */
int main(int argc, char *argv[])
{
    _EXPRESS_DEBUG_INSTRUCTION(openlog("Express-core", LOG_CONS, LOG_USER);) 

    try
    {
        pReactor = new Reactor(atoi(argv[3]));
    }
    catch (const std::system_error &err)
    {
        //TODO : Log error here.
        _EXPRESS_DEBUG_INSTRUCTION(syslog(LOG_DEBUG, "error for epoll : %s",err.what());)
    }
    RunController(atoi(argv[1]), atoi(argv[2]));
    return 0;
}

static void RunController(int master_channel, int heart_beat_rate)
{
    pMasterChannel = new UnixDomainSocket{master_channel};

    Signal(express::core::kWorkerTerminateSignal, TerminateHandler);
 
    for (;;)
    { 
        int connfd = pMasterChannel->ReceiveDescriptor();
        _EXPRESS_DEBUG_INSTRUCTION(syslog(LOG_DEBUG, "worker received a connection");)
        pReactor->Add(connfd);
        _EXPRESS_DEBUG_INSTRUCTION(syslog(LOG_DEBUG, "worker added a connection");)
    }
}

static void HeartBeat()
{
    // TODO : To be implemented.
}

static void TerminateHandler(int signo)
{
    delete pReactor;
    exit(0);
}