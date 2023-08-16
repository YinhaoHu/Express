#ifndef _EXPRESS_DATABASE_HANDLER_H
#define _EXPRESS_DATABASE_HANDLER_H

#include "utility/macro.h"
#include "utility/ipc/socket.h"
#include "utility/ipc/received_stream_message.h"
#include <memory>

_START_EXPRESS_NAMESPACE_

namespace database
{ 
    extern void handle(std::unique_ptr<utility::ipc::ReceivedStreamMessage> msg); 
}


_END_EXPRESS_NAMESPACE_

#endif