#ifndef _EXPRESS_UTILITY_IPC_UNIX_DOMAIN_SOCKET_H
#define _EXPRESS_UTILITY_IPC_UNIX_DOMAIN_SOCKET_H

#ifdef __linux

#include "utility/macro.h"
#include "utility/misc.h"
#include "utility/ipc/socket.h"
#include "utility/ipc/basic_socket.h"
#include <utility>
#include <stdexcept>

_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    /**
     * @brief A unix domain socket wrapper for Linux only.
     */
    class UnixDomainSocket : public BasicSocket
    {
        
    public:
        static std::pair<Socket, Socket> SocketPair(SocketType sock_type = SocketType::Stream);

    public:
        UnixDomainSocket(SocketType sock_type = SocketType::Stream);

        UnixDomainSocket(Socket sock_fd);

        /**
         * @throw  std::system_rror if error in system call.
         */
        void SendDescriptor(int fd);

        /**
         * @return The guaranteed valid descriptor if no exception thrown.
         * @throw
         * 1. std::runtime_error : Unexpected msghdr ancillary data.
         * 2. std::system_rror: Error in system call.
         */
        int ReceiveDescriptor();
    };
}

_END_EXPRESS_NAMESPACE_

#endif // for #ifdef __linux
#endif // for express macro