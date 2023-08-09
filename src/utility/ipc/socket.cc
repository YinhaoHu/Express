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
}