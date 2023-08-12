#ifndef _EXPRESS_UTILITY_IPC_TCP_SOCKET_H
#define _EXPRESS_UTILITY_IPC_TCP_SOCKET_H

#include "utility/macro.h"
#include "utility/data/rawdata.h"
#include "utility/ipc/basic_socket.h"
#include "utility/ipc/socket.h" 

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /**
         * @brief A tcp socket wrapper class.
         */
        class TCPSocket :public BasicSocket
        {
        public:
            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            TCPSocket(InternetProtocol ip_version)
                :BasicSocket(SystemIPConstant(ip_version), SOCK_STREAM)
            {

            }
            TCPSocket(Socket sockfd)
                :BasicSocket(sockfd)
            {

            }

            ~TCPSocket() = default;
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif