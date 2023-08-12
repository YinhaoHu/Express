#ifndef _EXPRESS_UTILITY_IPC_TCP_CLIENT_H
#define _EXPRESS_UTILITY_IPC_TCP_CLIENT_H

#include "utility/macro.h"
#include "utility/ipc/socket.h"
#include "utility/ipc/sent_message.h"
#include "utility/ipc/received_stream_message.h"
#include "utility/ipc/tcp_socket.h"


_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    class TCPClient
    {
    public:
        enum class Status{kDisconnected, kConnected};
    public:
        TCPClient(InternetProtocol ip_version, const char *ip_address, const char *port);
        ~TCPClient();

        /**
         * @throw std::runtime_error will be thrown. The specific information
         * can be known in the chosen message queue manual.
         */
        void Send(const SentMessage &msg);

        /**
         * @throw std::runtime_error will be thrown. The specific information
         * can be known in the chosen message queue manual.
         */
        std::unique_ptr<ReceivedStreamMessage> Receive();

        void Disconnect() noexcept;

        /**
         * @throw std::runtime_error will be thrown in the two cases.
         * Case1: The error from system related socket functions.
         * Case2: The client is connected to server.
         */
        void Reconnect();

        Status GetStatus();

    private:
        void ConnectToServer();

    private:
        TCPSocket socket_;
        Status status_;
        std::unique_ptr<std::string> spAddress_, spPort_;
        InternetProtocol ip_version_;
    };
}

_END_EXPRESS_NAMESPACE_

#endif