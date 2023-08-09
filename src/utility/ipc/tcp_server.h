#ifndef _EXPRESS_UTILITY_IPC_TCP_SERVER_H
#define _EXPRESS_UTILITY_IPC_TCP_SERVER_H

#include "utility/macro.h"
#include "tcp_socket.h"
#include <queue>


_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    class TCPServer
    {
        public:
            enum class Status{kToBeStarted,kRunning, kClosed};

        public:
            TCPServer(const char* port, InternetProtocol ip = InternetProtocol::kAny, 
                      int backlog = SOMAXCONN);
            ~TCPServer();

            Status GetStatus()const noexcept;
            
            bool HasPendingConnections();
            TCPSocket NextPending();

        private:
            std::queue<TCPSocket> pending_connections_;
            TCPSocket socket_;
            Status status_;
    };
}


_END_EXPRESS_NAMESPACE_

#endif