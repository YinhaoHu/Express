#ifndef _EXPRESS_UTILITY_IPC_SOCKET_H
#define _EXPRESS_UTILITY_IPC_SOCKET_H

#include "utility/macro.h"

#ifdef __linux__

using Socket = int;
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#else

using Socket = SOCKET;

#endif
 
_START_EXPRESS_NAMESPACE_


namespace utility::ipc
{ 
    enum class InternetProtocol{ kAny , kIPv4 ,kIPv6 };
    enum class SocketType{Stream , Datagram };

    int SystemIPConstant(InternetProtocol ip);
    int SystemSocketTypeConstant(SocketType type);
}


_END_EXPRESS_NAMESPACE_

#endif