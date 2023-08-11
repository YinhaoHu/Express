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
        : socket_(ip_version), status_(Status::kDisconnected)
    {
        struct addrinfo hint, *p, *list;

        memset(&hint, 0, sizeof(hint));
        hint.ai_family = SystemIPConstant(ip_version);
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

        if (getaddrinfo(ip_address, port, &hint, &list) < 0)
            throw runtime_error(ErrorString("TCPClient::TCPClient"));

        for (p = list; p != nullptr; p = p->ai_next)
        {
            socket_.Connect(*(p->ai_addr), p->ai_addrlen);
            break;
        }

        freeaddrinfo(list);
    }

    TCPClient::~TCPClient()
    {
        Disconnect();
    }
 
    void TCPClient::Send(const SentMessage &msg)
    { 
        socket_.Send(msg.Data().get(), msg.Size());
    }

    std::unique_ptr<ReceivedStreamMessage> TCPClient::Receive()
    { 
        shared_ptr<ReceivedStreamMessage::Header> spHeader;

        socket_.Receive(spHeader->GetData(), spHeader->Size(), MSG_WAITALL);

        ReceivedStreamMessage msg(spHeader);

        socket_.Receive(msg.GetBodyHandler().get(), spHeader->body_size, MSG_WAITALL);
        msg.ValidateBody();

        return std::make_unique<ReceivedStreamMessage>(msg);
    }

    void TCPClient::Disconnect()
    {
        if (status_ == Status::kConnected)
        {
            status_ = Status::kDisconnected;
            socket_.Close();
        }
    }

    TCPClient::Status TCPClient::GetStatus() const noexcept
    {
        return status_;
    }

}

_END_EXPRESS_NAMESPACE_