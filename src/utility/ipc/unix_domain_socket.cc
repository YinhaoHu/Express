#ifdef __linux
#include "unix_domain_socket.h"
#include <memory>

_START_EXPRESS_NAMESPACE_

using namespace std;

namespace utility::ipc
{
        UnixDomainSocket::  UnixDomainSocket(SocketType sock_type)
            : BasicSocket(AF_UNIX, SystemSocketTypeConstant(sock_type))
        { }

        UnixDomainSocket::  UnixDomainSocket(Socket sock_fd)
            : BasicSocket(sock_fd)
        { }

        std::pair<Socket, Socket>  UnixDomainSocket:: SocketPair(SocketType sock_type)
        {
            int sockfds[2];

            if (socketpair(AF_UNIX, SystemSocketTypeConstant(sock_type), 0, sockfds) < 0)
                throw std::runtime_error(misc::ErrorString("UnixDomainSocket::SocketPair"));

            return std::make_pair(sockfds[0], sockfds[1]);
        }

        void UnixDomainSocket:: SendDescriptor(int fd)
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
 
        int UnixDomainSocket:: ReceiveDescriptor()
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

        void UnixDomainSocket::SendMessage(const SentMessage& msg)
        {
            auto msg_data = msg.StreamData();
            size_t iovec_size = msg_data->size()*2 - 1;
            unique_ptr<iovec[]> spIovec = make_unique<iovec[]>(iovec_size); 

            for(size_t i = 0; i < iovec_size; ++i)
            {
                if(i % 2 == 0)
                { 
                    auto& item = msg_data->at(i/2);
                    spIovec[i].iov_base = const_cast<char*>(item.pData);
                    spIovec[i].iov_len = item.size;
                    if(i != 0)
                    {
                        spIovec[i-1].iov_base = &(item.size);
                        spIovec[i-1].iov_len = sizeof(item.size);
                    }
                } 
            }

            unique_ptr<msghdr> spMsg = make_unique<msghdr>();

            memset(spMsg.get(), 0, sizeof(msghdr));

            spMsg->msg_iov = spIovec.get();
            spMsg->msg_iovlen = iovec_size; 
            if(sendmsg(socket_, spMsg.get(), 0) < 0)
                misc::ThrowSystemError(SYSTEM_ERROR_INFO("UnixDomainSocket::SendMessage()"));
        }

        std::unique_ptr<ReceivedStreamMessage>  UnixDomainSocket::ReceiveMessage()
        {
            using Message = ReceivedStreamMessage;

            auto spHeader = make_shared<Message::Header>(); 
            Receive(spHeader.get(), spHeader->Size());

            auto spMsg = make_unique<Message>(spHeader);
            Receive(spMsg->GetBodyHandler().get(), spHeader->body_size);
            
            spMsg->ValidateBody();

            return spMsg;
        }
}

_END_EXPRESS_NAMESPACE_

#endif