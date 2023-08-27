#include "tcp_server.h"
#include "utility/misc.h"
#include <cstring>
#include <mutex>
#include <stdexcept>

using namespace std;
_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    TCPServer::TCPServer(const char *port, InternetProtocol ip,
                         int backlog)
        : socket_(ip), status_(Status::kToBeStarted)
    {
        using namespace misc;
        int enable_reuseaddr = 1;
        struct addrinfo hint, *result, *p; 

        memset(&hint, 0, sizeof(hint));
        hint.ai_family = SystemIPConstant(ip);
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_PASSIVE;
        if (getaddrinfo(nullptr, port, &hint, &result) < 0)
            throw runtime_error(ErrorString("TCPServer::TCPServer"));

        for (p = result; p != nullptr; p = p->ai_next)
        {
            socket_.SetOption(SO_REUSEADDR, &enable_reuseaddr, sizeof(enable_reuseaddr));

            socket_.Bind(*(p->ai_addr));

            break;
        }

        socket_.Listen();
        freeaddrinfo(result);

        status_ = Status::kRunning;
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
        }
        else
        {
            throw std::runtime_error("TCPServer::Close : Server is not running.");
        }
    }
    
    std::unique_ptr<TCPSocket> TCPServer::NextPending()
    {
        if (status_ != Status::kRunning)
            throw runtime_error("Server is not running now.");
        else
        {
            auto res = pending_connections_.front();
            this->pending_connections_.pop();

            return std::unique_ptr<TCPSocket>(res);
        }
    }

    bool TCPServer::WaitForConnection(int timeout_msec)
    {
        struct timeval old_timeout, new_timeout;
        socklen_t old_timeout_len;

        if (timeout_msec != -1)
        {
            new_timeout.tv_sec = 0;
            new_timeout.tv_usec = 1000 * timeout_msec;
            socket_.GetOption(SO_RCVTIMEO, &old_timeout, &old_timeout_len);
            socket_.SetOption(SO_RCVTIMEO, &new_timeout, sizeof(new_timeout));
        }

        bool has_new_conn = false;
        try
        {
            Socket connfd = socket_.Accept();
            TCPSocket* new_conn = new TCPSocket(connfd);

            has_new_conn = true;
            pending_connections_.emplace(new_conn);
        }
        catch (const std::system_error &err)
        {
            if (err.code().value() != EWOULDBLOCK && err.code().value() != EAGAIN)
                throw err;
        }
        if (timeout_msec != -1)
            socket_.SetOption(SO_RCVTIMEO, &old_timeout, old_timeout_len);

        return has_new_conn;
    }

}

_END_EXPRESS_NAMESPACE_