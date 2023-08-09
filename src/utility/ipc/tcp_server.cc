#include "tcp_server.h"
#include "utility/misc.h"
#include <cstring>
#include <stdexcept>

using namespace std;
_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    TCPServer::TCPServer(const char *port, InternetProtocol ip = InternetProtocol::kAny,
                         int backlog = SOMAXCONN)
        : status_(Status::kToBeStarted), socket_(ip)
    {
        using namespace misc;
        int enable_reuseaddr = 1;
        struct addrinfo hint, *result, *p;

        memset(&hint, 0, sizeof(hint));
        hint.ai_family = SystemIPConstant(ip);
        hint.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(nullptr, port, &hint, &result) < 0)
            throw runtime_error(ErrorString("TCPServer::TCPServer"));

        // TODO finish the rest.
        for (p = result; p != nullptr; p = p->ai_next)
        {
            socket_.SetOption(SO_REUSEADDR, &enable_reuseaddr, sizeof(enable_reuseaddr));

            socket_.Bind(static_cast<sockaddr>(*(p->ai_addr)), p->ai_addrlen);
        }

        socket_.Listen();
        freeaddrinfo(result);
        status_ = Status::kRunning;
        
        auto accept_entry = [this]()
        {
            auto& sock = this->socket_;

            auto new_conn = sock.Accept();
        };

        paccept_thread_ = new thread();
    }

    TCPServer::~TCPServer()
    {
        if (status_ == Status::kRunning)
            socket_.Close();
    }

    TCPServer::Status TCPServer::GetStatus() const noexcept
    {
        return status_;
    }

    void TCPServer::Close()
    {
        if (status_ == Status::kRunning)
        {
            status_ = Status::kClosed;
            socket_.Close();
            paccept_thread_->join();
        }
        else 
        {
            throw std::runtime_error("TCPServer::Close : Server is not running.");
        }
    }

    bool TCPServer::HasPendingConnections()
    {
        return !pending_connections_.empty();
    }


    // TODO Implement this function.
    TCPSocket TCPServer::NextPending() 
    {
        
    }

}

_END_EXPRESS_NAMESPACE_