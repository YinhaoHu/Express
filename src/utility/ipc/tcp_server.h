#ifndef _EXPRESS_UTILITY_IPC_TCP_SERVER_H
#define _EXPRESS_UTILITY_IPC_TCP_SERVER_H

#include "utility/macro.h"
#include "tcp_socket.h"
#include <shared_mutex>
#include <thread>
#include <memory>
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
        TCPServer(const TCPServer& ) = delete;
        ~TCPServer();

        Status GetStatus() const noexcept;
        void Close() ;

        /**
         * @note The returned TCP Socket should be closed automatically.
        */
        std::unique_ptr<TCPSocket> NextPending();
        bool WaitForConnection(int timeout_msec = -1);

    private: 
        std::queue<TCPSocket*> pending_connections_;
        TCPSocket socket_;
        Status status_;
    };
}

_END_EXPRESS_NAMESPACE_

#endif