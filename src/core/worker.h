#ifndef _EXPRESS_CORE_WORKER_H
#define _EXPRESS_CORE_WORKER_H

#include <string>

#include "common.h"
#include "utility/misc.h"
#include "utility/ipc/unix_domain_socket.h"
#include "utility/os/local_process.h"
#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace core
{
    class Worker : public utility::os::LocalProcess
    {
    public:
        Worker(int hear_beat_rate, int db_channel);
        Worker(const Worker &) = delete;
        ~Worker();

        void AddTask(int connfd); 

    private:
        utility::ipc::UnixDomainSocket *pCallerSocket_, *pDBSocket_;
        int heart_beat_rate_;
    };

    inline Worker::Worker(int heart_beat_rate, int db_channel)
        : pCallerSocket_(nullptr),
          pDBSocket_(new utility::ipc::UnixDomainSocket(db_channel)),
          heart_beat_rate_(heart_beat_rate)
    {
        using utility::ipc::UnixDomainSocket;
        auto sock_pair = UnixDomainSocket::SocketPair();

        pCallerSocket_ = new UnixDomainSocket(sock_pair.first);

        pid_ = fork();
        if (pid_ < 0)
            utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("LocalProcess::ForkAndExecute"));
        else if (pid_ == 0)
        {
            pCallerSocket_->Close();
            if (execl(worker_exe_name.data(), "worker",
                             std::to_string(sock_pair.second).c_str(),
                             std::to_string(heart_beat_rate).c_str(),
                             std::to_string(db_channel).c_str(), nullptr) < 0
                )
                utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("LocalProcess::ForkAndExecute"));
        } 
        close(sock_pair.second);
    }

    inline Worker::~Worker()
    {
        if (pid_ != getpid())
        {
            pCallerSocket_->Close();
            pDBSocket_->Close();

            delete pCallerSocket_;
            delete pDBSocket_;
        }
    }

    inline void Worker::AddTask(int connfd)
    {
        _EXPRESS_DEBUG_INSTRUCTION(syslog(LOG_DEBUG, "master added a task.");)
        pCallerSocket_->SendDescriptor(connfd);
    }

}

_END_EXPRESS_NAMESPACE_

#endif