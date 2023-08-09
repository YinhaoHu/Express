#ifndef _EXPRESS_UTILITY_IPC_TCP_SERVER_H
#define _EXPRESS_UTILITY_IPC_TCP_SERVER_H

#include "utility/macro.h"
#include "tcp_socket.h"
#include <mutex>
#include <thread>
#include <queue>

_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    class TCPServer
    {
    public:
        enum class Status
        {
            kToBeStarted,
            kRunning,
            kClosed
        };

    public:
        /**
         * @throw std::runtime_error will be thrown when system call set errno.
         */
        TCPServer(const char *port, InternetProtocol ip = InternetProtocol::kAny,
                  int backlog = SOMAXCONN);
        ~TCPServer();

        Status GetStatus() const noexcept;
        void Close() ;

        bool HasPendingConnections();
        TCPSocket NextPending();

    private:
        std::thread* paccept_thread_;
        std::mutex pending_connections_mutex_;
        std::queue<TCPSocket> pending_connections_;
        TCPSocket socket_;
        Status status_;
    };
}

_END_EXPRESS_NAMESPACE_

#endif