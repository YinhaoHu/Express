#include "tcp_client.h"
#include "utility/misc.h"
#include <cstring>
#include <stdexcept>

_START_EXPRESS_NAMESPACE_
using namespace std;
using namespace utility::misc;

namespace utility::ipc
{
    TCPClient::TCPClient(InternetProtocol ip_version, const char *ip_address, const char *port)
        : socket_(ip_version), status_(Status::kDisconnected),
        spAddress_(new string(ip_address)), spPort_(new string(port)),
        ip_version_(ip_version)
    { 
        ConnectToServer();
    }

    TCPClient::~TCPClient()
    {
        Disconnect();
    }
 
    void TCPClient::Send(const SentMessage &msg)
    { 
        socket_.SendMessage(msg);
    }

    std::unique_ptr<ReceivedStreamMessage> TCPClient::Receive()
    { 
        return socket_.ReceiveMessage();
    }

    void TCPClient::Disconnect() noexcept
    {
        if (status_ == Status::kConnected)
        {
            status_ = Status::kDisconnected;
            socket_.Close();
        }
    }

    void TCPClient::Reconnect()
    {
        ConnectToServer();
    }

    TCPClient::Status TCPClient::GetStatus()
    {
        char buf[1];
        ssize_t res;

        timeval old_timeout, new_timeout;
        socklen_t old_timeout_size = sizeof(old_timeout);

        new_timeout.tv_sec = 1;
        new_timeout.tv_usec = 0;

        socket_.GetOption(SO_RCVTIMEO, &old_timeout, &old_timeout_size);
        socket_.SetOption(SO_RCVTIMEO, &new_timeout, sizeof(new_timeout));

        errno = 0;
        res = recv(socket_.GetNative(), buf, 1, MSG_PEEK);

        if(res < 0 &&  errno != EAGAIN && errno != EWOULDBLOCK )
            throw runtime_error(ErrorString("TCPClient::GetStatus()"));
        else if(res == 0)
            status_ = Status::kDisconnected; 

        socket_.SetOption(SO_RCVTIMEO, &old_timeout, sizeof(old_timeout));

        return status_;
    }


    void TCPClient::ConnectToServer()
    {
        if(status_ == Status::kConnected)
            throw runtime_error("Client has been connected to server.");

        struct addrinfo hint, *p, *list;

        memset(&hint, 0, sizeof(hint));
        hint.ai_family = SystemIPConstant(ip_version_);
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

        if (getaddrinfo(spAddress_->c_str(), spPort_->c_str(), &hint, &list) < 0)
            throw runtime_error(ErrorString("TCPClient::ConnectToServer"));

        for (p = list; p != nullptr; p = p->ai_next)
        {
            socket_.Connect(*(p->ai_addr));
            break;
        }

        freeaddrinfo(list);
        status_ = Status::kConnected;
    }
}   

_END_EXPRESS_NAMESPACE_