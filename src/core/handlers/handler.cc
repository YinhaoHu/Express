#include "handler.h"

#include <functional>

#include "utility/ipc/sent_message.h"
#include "utility/ipc/tcp_socket.h" 
#include "database/client.h"

_START_EXPRESS_NAMESPACE_

using utility::ipc::TCPSocket;
using utility::ipc::ReceivedStreamMessage;
using utility::ipc::SentMessage;

namespace core
{
    /*          Handlers List - begin            */ 
    extern HandlerType handler0_pingpong ;
    /*          Handlers List - end              */

    static HandlerInfrastructure* pInfra;
    static std::vector<HandlerType>* pHandlers;

    void InitHandler(int db_channel) 
    {
        pInfra = new HandlerInfrastructure( new database::Client(db_channel));

        pHandlers = new std::vector<HandlerType>
        {
            handler0_pingpong,
        }; 
    }

    void Handle(TCPSocket &client,ReceivedStreamMessage &request)
    { 
        // TODO : Consider log the time for one service information here
        // including the time.
        using HdrField = ReceivedStreamMessage::Header::Field; 
        uint32_t comm_code = request.GetHeaderField(HdrField::kCommunicationCode); 
        pHandlers->at(comm_code)(client, request, *pInfra);
    }
} // namespace core




_END_EXPRESS_NAMESPACE_