#ifndef _EXPRESS_UTILITY_IPC_UNIX_DOMAIN_SOCKET_H
#define _EXPRESS_UTILITY_IPC_UNIX_DOMAIN_SOCKET_H

#include "utility/macro.h"
#include "utility/misc.h"
#include "utility/ipc/socket.h"
#include "utility/ipc/basic_socket.h"
#include <utility>
#include <stdexcept>

_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    class UnixDomainSocket : public BasicSocket
    {
    public:
        UnixDomainSocket(SocketType sock_type)
            : BasicSocket(AF_UNIX, SystemSocketTypeConstant(sock_type))
        { }

        UnixDomainSocket(Socket sock_fd)
            : BasicSocket(sock_fd)
        { }

        static std::pair<Socket, Socket> SocketPair(SocketType sock_type)
        {
            int sockfds[2];

            if (socketpair(AF_UNIX, SystemSocketTypeConstant(sock_type), 0, sockfds) < 0)
                throw std::runtime_error(misc::ErrorString("UnixDomainSocket::SocketPair"));

            return std::make_pair(sockfds[0], sockfds[1]);
        }

#ifdef __linux
        /** 
         * @throw  std::system_rror if error in system call.
        */
        void SendDescriptor(int fd)
        {
            struct msghdr msg;
            struct cmsghdr *cmptr;
            struct iovec vec;
            char io_buf[1];

            vec.iov_base = io_buf;
            vec.iov_len = 1;

            msg.msg_name = nullptr;
            msg.msg_namelen = 0;
            msg.msg_iov = &vec;
            msg.msg_iovlen = 1;
            msg.msg_flags = 0;

            constexpr size_t control_size = CMSG_SPACE(sizeof(int));
            union {
                struct cmsghdr cm;
                char control[control_size];
            }control_un;
            
            msg.msg_controllen = sizeof(control_un.control);
            msg.msg_control = control_un.control;

            cmptr = CMSG_FIRSTHDR(&msg);
            cmptr->cmsg_level = SOL_SOCKET;
            cmptr->cmsg_type = SCM_RIGHTS;
            cmptr->cmsg_len = CMSG_LEN(sizeof(int));
            *reinterpret_cast<int*>CMSG_DATA(cmptr) = fd;
 
            if(sendmsg(socket_, &msg, 0) < 0)
                misc::ThrowSystemError(SYSTEM_ERROR_INFO("UnixDomainSocket::SendDescriptor"));

        }

        /**
         * @return The guaranteed valid received descriptor if no exception thrown.
         * @throw 
         * 1. std::runtime_error : Unexpected msghdr ancillary data.
         * 2. std::system_rror: Error in system call.
        */
        int ReceiveDescriptor()
        {
            struct msghdr msg;
            struct cmsghdr *cmptr;
            struct iovec vec;
            char io_buf[1];

            vec.iov_base = io_buf;
            vec.iov_len = 1;

            msg.msg_name = nullptr;
            msg.msg_namelen = 0;
            msg.msg_iov = &vec;
            msg.msg_iovlen = 1;
            msg.msg_flags = 0;

            constexpr size_t control_size = CMSG_SPACE(sizeof(int));
            union {
                struct cmsghdr cm;
                char control[control_size];
            }control_un;

            msg.msg_control = control_un.control;
            msg.msg_controllen = sizeof(control_un.control);

            if(recvmsg(socket_, &msg, 0) < 0 ) 
                misc::ThrowSystemError(SYSTEM_ERROR_INFO("UnixDomainSocket::ReceiveDescriptor"));
 
            cmptr = CMSG_FIRSTHDR(&msg);
            if( cmptr != nullptr && cmptr->cmsg_len == CMSG_LEN(sizeof(int))
                &&cmptr->cmsg_level == SOL_SOCKET &&cmptr->cmsg_type == SCM_RIGHTS)
            {  
                return *reinterpret_cast<int*>CMSG_DATA(cmptr);
            }

            throw std::runtime_error("UnixDomainSocket::ReceiveDescriptor : No control header.");
        }
#endif
    };
}

_END_EXPRESS_NAMESPACE_

#endif