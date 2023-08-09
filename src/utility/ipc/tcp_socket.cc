#include "tcp_socket.h"
#include "utility/misc.h"
#include <stdexcept>
#include <cstring>

_START_EXPRESS_NAMESPACE_

using namespace utility::misc;
using namespace std;

namespace utility::ipc
{
    TCPSocket::TCPSocket(InternetProtocol ip_version)
        :socket_(0), closed_(false)
    {
        int domain;
        switch (ip_version)
        {
        case InternetProtocol::kAny:
            domain = AF_UNSPEC;
            break;
        case InternetProtocol::kIPv4:
            domain = AF_INET;
            break;
        case InternetProtocol::kIPv6:
            domain = AF_INET6;
            break;
        }
        socket_ = socket(domain, SOCK_STREAM, 0);
        if (socket_ < 0)
            throw runtime_error(ErrorString("TCPSocket::TCPSocket"));
    }

    TCPSocket::TCPSocket(Socket sockfd)
        :socket_(sockfd), closed_(false)
    {}

    TCPSocket::~TCPSocket()
    {
        if (!closed_)
            this->Close();
    }
    void TCPSocket::Close()
    {
        closed_ = true;
        close(socket_);
    }

    ssize_t TCPSocket::Send(const char *data, size_t size, int flag = 0)
    {
        ssize_t sent_bytes = send(socket_, data, size, flag);

        if (sent_bytes < 0)
            throw runtime_error(ErrorString("TCPSocket::Send"));
        return sent_bytes;
    }
    ssize_t TCPSocket::Receive(void *buf, size_t size, int flag = 0)
    {
        ssize_t recv_bytes = recv(socket_, buf, size, flag);

        if (recv_bytes < 0)
            throw runtime_error(ErrorString("TCPSocket::Send"));
        return recv_bytes;
    }

    void TCPSocket::SetOption(int opt, const void *pval, socklen_t len)
    {
        int rc;
        rc = setsockopt(socket_, SOL_SOCKET, opt, pval, len);
        if (rc < 0)
            throw(runtime_error(ErrorString("TCPSocket::SetOption")));
    }

    socklen_t TCPSocket::GetOption(int opt, void *pval)
    {
        socklen_t len;
        int rc;

        rc = getsockopt(socket_, SOL_SOCKET, opt, pval, &len);
        if (rc < 0)
            throw runtime_error(ErrorString("TCPSocket::GetOption"));
        return len;
    }

    inline Socket TCPSocket::GetSocket() const noexcept {
        return this->socket_;
    }

    void TCPSocket::Connect(const sockaddr &addr, socklen_t addr_len) 
    {
        if(connect(socket_, &addr, addr_len) < 0)
            throw runtime_error("TCPSocket::Connect");
    }
    void TCPSocket::Bind(const sockaddr &addr, socklen_t addr_len) 
    {
        if(bind(socket_, &addr, addr_len) < 0)
            throw runtime_error("TCPSocket::Bind");
    }
    void TCPSocket::Listen(int backlog = SOMAXCONN) 
    {
        if(listen(socket_, backlog) < 0)
            throw runtime_error("TCPSocket::Listen");
    }
}

_END_EXPRESS_NAMESPACE_