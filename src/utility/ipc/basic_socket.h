#ifndef _EXPRESS_UTILITY_IPC_BASIC_SOCKET_H
#define _EXPRESS_UTILITY_IPC_BASIC_SOCKET_H

#include "utility/macro.h"
#include "utility/misc.h"
#include "utility/ipc/socket.h"

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        class BasicSocket
        {
        public:
            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            BasicSocket(int domain, int type);
            BasicSocket(Socket sockfd);
            virtual ~BasicSocket();

            void Close();

            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            ssize_t Send(const char *buf, size_t size, int flag = 0);

            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            ssize_t Receive(void *buf, size_t size, int flag = 0);

            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            void SetOption(int opt, const void *pval, socklen_t len);

            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            void GetOption(int opt, void *pval, socklen_t *len);

            Socket GetNative() const noexcept;

            /**
             * @throw std::system_error will be thrown when system call set errno.
             */
            template <typename SocketAddressT>
            void Connect(const SocketAddressT &socket_addr)
            {
                if (connect(socket_, reinterpret_cast<const sockaddr *>(&socket_addr), sizeof(SocketAddressT)) < 0)
                   misc::ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Connect"));
            }

            /**
             * @throw std::system_error will be thrown when system call set errno.
             */
            template <typename SocketAddressT>
            void Bind(const SocketAddressT &socket_addr)
            {
                if (bind(socket_, reinterpret_cast<const sockaddr *>(&socket_addr), sizeof(SocketAddressT)) < 0)
                    misc::ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Bind"));
            }
            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            void Listen(int backlog = SOMAXCONN);

            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            Socket Accept();

            /**
             * @note If this function is called in Message transfer, you would set
             * TCP_CORK option before using. The recommended way is that set include
             * tail files in the SentMessage.
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            void SendFile(const char *file_path);

        protected:
            bool closed_;
            Socket socket_;
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif