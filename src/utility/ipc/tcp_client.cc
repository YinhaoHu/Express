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
        const auto& spData = * msg.StreamData();  

        socket_.Send(spData[0].pData, spData[0].size, MSG_MORE);

        size_t last_idx = spData.size() - 1;
        for(size_t i = 1; i < last_idx; ++i)
        {
            socket_.Send(reinterpret_cast<const char*>(&(spData[i].size)), 
                sizeof(spData[i].size), MSG_MORE);
            socket_.Send(spData[i].pData, spData[i].size,  MSG_MORE );
        }
        
        socket_.Send(reinterpret_cast<const char*>(&(spData[last_idx].size)), 
            sizeof(spData[last_idx].size), MSG_MORE);
        socket_.Send(spData[last_idx].pData, spData[last_idx].size, MSG_WAITALL);
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