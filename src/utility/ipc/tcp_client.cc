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
            socket_.Connect(*(p->ai_addr), sizeof(p->ai_addrlen));
            break;
        }

        freeaddrinfo(list);
    }

    TCPClient::~TCPClient()
    {
        Disconnect();
    }
 
    void TCPClient::Send(const Message &msg)
    {
        auto size = msg.Size();
        auto data = msg.Rawdata().get();
        socket_.Send(data, size);
    }

    std::unique_ptr<Message> TCPClient::Receive()
    { 
        auto header_size = Message::Header::Size();
        char* header_buf = new char[header_size];

        socket_.Receive(header_buf,header_size);
        unique_ptr<Message> res(make_unique<Message>(header_buf));

        auto body_size = res->GetHeaderField(Message::HeaderField::KBodySize);

        char* body_buf = new char[body_size];
        res->SetFields(body_buf);

        return res;
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