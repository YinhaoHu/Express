#ifndef _EXPRESS_UTILITY_IPC_SOCKET_H
#define _EXPRESS_UTILITY_IPC_SOCKET_H

#include "utility/macro.h"

#ifdef __linux__

using Socket = int;
#include <sys/socket.h>
#include <netdb.h>

#else

using Socket = SOCKET;

#endif
 
_START_EXPRESS_NAMESPACE_


namespace utility::ipc
{ 
    enum class InternetProtocol{ kAny, kIPv4,kIPv6};

    int SystemIPConstant(InternetProtocol ip);
}


_END_EXPRESS_NAMESPACE_

#endif