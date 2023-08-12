#include "tcp_server.h"
#include "utility/misc.h"
#include <cstring>
#include <mutex>
#include <stdexcept>

using namespace std;
_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    TCPServer::TCPServer(const char *port, InternetProtocol ip ,
                         int backlog, int accept_timeout)
        :socket_(ip), status_(Status::kToBeStarted)
    {
        using namespace misc;
        int enable_reuseaddr = 1;
        struct addrinfo hint, *result, *p;
        timeval timeout;

        memset(&hint, 0, sizeof(hint));
        hint.ai_family = SystemIPConstant(ip);
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_PASSIVE ;
        if (getaddrinfo(nullptr, port, &hint, &result) < 0)
            throw runtime_error(ErrorString("TCPServer::TCPServer"));

        for (p = result; p != nullptr; p = p->ai_next)
        {
            timeout.tv_sec = accept_timeout;
            timeout.tv_usec = 0;

            socket_.SetOption(SO_RCVTIMEO, &timeout, sizeof(timeout));
            socket_.SetOption(SO_REUSEADDR, &enable_reuseaddr, sizeof(enable_reuseaddr));

            socket_.Bind(*(p->ai_addr));

            break;
        }

        socket_.Listen();
        freeaddrinfo(result);

        status_ = Status::kRunning;

        auto accept_entry = [this]()
        {
            auto &sock = this->socket_;

            auto new_connection = [&]() -> auto
            {
                Socket new_sock;
                new_sock = sock.Accept();
                TCPSocket *new_conn = new TCPSocket(new_sock);
                return new_conn;
            };

            for (;;)
            { 
                if(this->status_ != Status::kRunning)
                    break;

                TCPSocket* new_con;
                try
                {  
                    new_con = new_connection();
                }
                catch(const std::runtime_error& e)
                {
                    if(errno == EAGAIN)
                        continue;
                    else 
                        throw e;
                }
                 
                if (this->status_ == Status::kRunning)
                { 
                    unique_lock lock(this->pending_connections_mutex_);
                    this->pending_connections_.push(new_con);
                }
                else 
                {
                    new_con->Close();
                }
            }
        };

        paccept_thread_ = new thread(accept_entry);
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
        if (status_ == Status::kRunning)
        {
            std::shared_lock lock(this->pending_connections_mutex_);
            return !pending_connections_.empty();
        }
        else
            throw runtime_error("Server is not running now.");
    }

    std::unique_ptr<TCPSocket> TCPServer::NextPending()
    {
        if (status_ != Status::kRunning)
            throw runtime_error("Server is not running now.");
        else
        {
            shared_lock lock(this->pending_connections_mutex_);

            auto res = pending_connections_.front();

            this->pending_connections_.pop();

            return std::unique_ptr<TCPSocket>(res);
        }
    }

}

_END_EXPRESS_NAMESPACE_