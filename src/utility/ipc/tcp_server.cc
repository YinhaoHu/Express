#include "tcp_server.h"
#include "utility/misc.h"
#include <cstring>
#include <stdexcept>


using namespace std;
_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    TCPServer::TCPServer(const char* port, InternetProtocol ip = InternetProtocol::kAny,
        int backlog = SOMAXCONN)
        :status_(Status::kToBeStarted), socket_(ip)
    {
        using namespace misc;
        struct addrinfo hint, *result,*p;

        memset(&hint, 0, sizeof(hint));
        switch(ip)
        {
            case InternetProtocol::kAny:
                hint.ai_family = AF_UNSPEC; break;
            case InternetProtocol::kIPv4:
                hint.ai_family = AF_INET; break;
            case InternetProtocol::kIPv6:
                hint.ai_family = AF_INET6; break;
        }
        hint.ai_socktype = SOCK_STREAM;
        if( getaddrinfo(nullptr, port, &hint, &result) < 0)
            throw runtime_error(ErrorString("TCPServer::TCPServer"));
        
        // TODO finish the rest.
        for(p = result; p != nullptr; p = p->ai_next)
        {
            
        }
        

        freeaddrinfo(result);
    }

    TCPServer::~TCPServer() 
    {
        if(status_ == Status::kRunning)
            socket_.Close();
    }

    TCPServer::Status TCPServer::GetStatus() const noexcept {}

    bool TCPServer::HasPendingConnections() {}
    TCPSocket TCPServer::NextPending() {}
}

_END_EXPRESS_NAMESPACE_