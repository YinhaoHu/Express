#include "socket.h"


namespace express::utility::ipc
{
    int SystemIPConstant(InternetProtocol ip)
    {
        int af;
        switch(ip)
        {
            case InternetProtocol::kAny:
                af = AF_UNSPEC; 
                break;
            case InternetProtocol::kIPv4:
                af = AF_INET; 
                break;
            case InternetProtocol::kIPv6:
                af = AF_INET6; 
                break;
        }
        return af;
    }

    int SystemSocketTypeConstant(SocketType type)
    {
        int sock_type;
        switch(type)
        {
            case SocketType::Stream:
                sock_type = SOCK_STREAM;
                break;
            case SocketType::Datagram:
                sock_type = SOCK_DGRAM;
                break;
        }
        return sock_type;
    }
}