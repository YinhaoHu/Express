#include "basic_socket.h"
#include "utility/misc.h"
#include <cstring>
#include <stdexcept>
#include <memory>
#include <sys/stat.h>

_START_EXPRESS_NAMESPACE_

using namespace utility::misc;
using namespace std;

namespace utility::ipc
{
    BasicSocket::BasicSocket(int domain, int type)
        : closed_(false), socket_(0) 
    {
        socket_ = socket(domain, type, 0);
        if (socket_ < 0)
            ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::BasicSocket()"));
    }

    BasicSocket::BasicSocket(Socket sockfd)
        : closed_(false), socket_(sockfd)
    {
    }

    BasicSocket::~BasicSocket()
    {
        if (!closed_)
            this->Close();
    }
    void BasicSocket::Close()
    {
        closed_ = true;
        close(socket_);
    }

    void BasicSocket::Send(const char *data, size_t size, int flag)
    {
        if(size == 0)
            throw invalid_argument("BasicSocket::Send(): Zero size is prohibited.");

        ssize_t once_sent_bytes = 0, total_sent_bytes = 0;

        while(total_sent_bytes != static_cast<ssize_t>(size))
        {
            once_sent_bytes  = send(socket_, data, size - total_sent_bytes, flag);
            if(once_sent_bytes < 0)
                misc::ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Send()"));
            total_sent_bytes += once_sent_bytes;
        } 
    }

    void BasicSocket::Receive(void *buf, size_t size, int flag)
    {
        if(size == 0)
            throw invalid_argument("BasicSocket::Send(): Zero size is prohibited.");

        ssize_t once_recv_bytes = 0, total_recv_bytes = 0;

        while(total_recv_bytes != static_cast<ssize_t>(size))
        {
            once_recv_bytes  = recv(socket_, buf, size - total_recv_bytes, flag);
            if(once_recv_bytes < 0)
                misc::ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Receive()"));
            total_recv_bytes += once_recv_bytes;
        } 
    }

    void BasicSocket::SetOption(int opt, const void *pval, socklen_t len)
    {
        int rc;
        rc = setsockopt(socket_, SOL_SOCKET, opt, pval, len);
        if (rc < 0)
            ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::SetOption()"));
    }

    void BasicSocket::GetOption(int opt, void *pval, socklen_t* len)
    {  
        int rc;
        rc = getsockopt(socket_, SOL_SOCKET, opt, pval,  len);
        if (rc < 0)
            ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::GetOption()")); 
    }

    Socket BasicSocket::GetNative() const noexcept
    {
        return this->socket_;
    }
 
 
    void BasicSocket::Listen(int backlog)
    {
        if (listen(socket_, backlog) < 0)
            ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Listen()"));
    }

    Socket BasicSocket::Accept()
    {
        Socket conncection_sockfd = accept(socket_, nullptr, nullptr);
        if (conncection_sockfd < 0)
             ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Accept"));
        return conncection_sockfd;
    }

    void BasicSocket::SendFile(const char *name )
    {
#ifdef __linux

        int in_fd = open(name, O_RDONLY);
        off_t offset = 0;
        unique_ptr<struct stat> spStat = make_unique<struct stat>();
        ssize_t sent_size, res;
 

        if (in_fd < 0)
             ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Sendfile-open"));

        memset(spStat.get(), 0, sizeof(struct stat));
        if (fstat(in_fd, spStat.get()) < 0)
             ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Sendfile-fstat"));

        while (sent_size != spStat->st_size)
        {
            res = sendfile(this->socket_, in_fd, &offset, spStat->st_size - offset);
            if (res < 0)
                 ThrowSystemError(SYSTEM_ERROR_INFO("BasicSocket::Sendfile-sednfile"));
            sent_size += res;
        }

        close(in_fd);

#else
#error "BasicSocket::Sendfile: Not implemented for Windows yet."
#endif
    }
}

_END_EXPRESS_NAMESPACE_