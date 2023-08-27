#ifndef _EXPRESS_CORE_HANDLERS_HANDLER_H
#define _EXPRESS_CORE_HANDLERS_HANDLER_H

#include <functional>

#include "utility/ipc/sent_message.h"
#include "utility/ipc/received_stream_message.h"
#include "utility/ipc/tcp_socket.h"
#include "utility/macro.h"
#include "database/client.h"

_START_EXPRESS_NAMESPACE_

namespace core
{
    struct HandlerInfrastructure{
        database::Client* pDataBase;
        // TODO: Log component will be here.

        HandlerInfrastructure(database::Client* _pDataBase)
        :pDataBase(_pDataBase){}

        HandlerInfrastructure(const HandlerInfrastructure&) = delete;

        ~HandlerInfrastructure()
        {
            delete pDataBase;
        }
    };
    using HandlerType = std::function<void(utility::ipc::TCPSocket&, 
                                            utility::ipc::ReceivedStreamMessage&,
                                            HandlerInfrastructure&
                                            )>;

    void InitHandler(int db_channel);
    void Handle(utility::ipc::TCPSocket &client, utility::ipc::ReceivedStreamMessage &request);
}

_END_EXPRESS_NAMESPACE_

#endif
