#ifndef _EXPRESS_UTILITY_IPC_TCP_SOCKET_H
#define _EXPRESS_UTILITY_IPC_TCP_SOCKET_H

#include "utility/macro.h"
#include "utility/data/rawdata.h"
#include "utility/ipc/socket.h"
#include <memory>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /**
         * @brief A tcp socket wrapper class. 
        */
        class TCPSocket
        {
        public:
            TCPSocket(InternetProtocol ip_version);
            TCPSocket(Socket sockfd);
            
            ~TCPSocket();
            void Close();

            ssize_t Send(const char *buf, size_t size, int flag = 0);
            ssize_t Receive(void *buf, size_t size, int flag = 0);

            void SetOption(int opt, const void *pval, socklen_t len);
            socklen_t GetOption(int opt, void *pval);

            Socket GetSocket() const noexcept;

            void Connect(const sockaddr &addr, socklen_t addr_len);
            void Bind(const sockaddr &addr, socklen_t addr_len);
            void Listen(int backlog = SOMAXCONN);

        private:
            bool closed_;
            Socket socket_;
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif