#ifndef _EXPRESS_UTILITY_IPC_BASIC_SOCKET_H
#define _EXPRESS_UTILITY_IPC_BASIC_SOCKET_H

#include "utility/macro.h"
#include "utility/misc.h"
#include "utility/ipc/socket.h"
#include <mutex>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /**
         * @brief A basic socket class.
         * 
         * @note Socket will not be closed automatically. This is reasonable.
        */
        class BasicSocket
        {
        public:
            /**
             * @throw std::system_error will be thrown when system call set errno.
             */
            BasicSocket(int domain, int type);
            BasicSocket(Socket sockfd);
            BasicSocket(const BasicSocket&) = delete;
            virtual ~BasicSocket();

            void Close();

            /**
             * @note size bytes are guaranteed to be sent if no exception thrown.
             * @throw Exceptions will be thrown in the two cases. 
             * 1. std::system_error will be thrown when system call set errno.
             * 2. std::invalid_argument will be thrown when size is zero.
             */
            void Send(const char *buf, size_t size, int flag = 0);

            /**
             * @note size bytes are guaranteed to be received if no exception thrown.
             * @throw Exceptions will be thrown in the two cases.
             * 1. std::system_error will be thrown when system call set errno.
             * 2. std::invalid_argument will be thrown when size is zero.
             */
            void Receive(void *buf, size_t size, int flag = MSG_WAITALL);

            /**
             * @throw std::system_error will be thrown when system call set errno.
             */
            void SetOption(int opt, const void *pval, socklen_t len);

            /**
             * @throw std::system_error will be thrown when system call set errno.
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
             * @throw std::system_error will be thrown when system call set errno.
             */
            void Listen(int backlog = SOMAXCONN);

            /**
             * @throw std::system_error will be thrown when system call set errno.
             */
            Socket Accept();

            /**
             * @note If this function is called in Message transfer, you would set
             * TCP_CORK option before using. The recommended way is that set include
             * tail files in the SentMessage.
             * @throw std::system_error will be thrown when system call set errno.
             */
            void SendFile(const char *file_path);

            std::mutex& GetWriteMutex();
            
            std::mutex& GetReadMutex();

        protected:
            bool closed_;
            Socket socket_;
        
        private:
            std::mutex read_mutex_, write_mutex_;
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif