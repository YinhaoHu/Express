/**
 * @file pingping.cc
 * @author Hoo
 * @brief A testing handler.
*/
#include <functional>

#include "handler.h"
#include "utility/macro.h"
#include "utility/ipc/tcp_socket.h"
#include "utility/ipc/received_stream_message.h"
#include "utility/ipc/sent_message.h" 

_START_EXPRESS_NAMESPACE_

using utility::ipc::TCPSocket;
using utility::ipc::ReceivedStreamMessage;
using utility::ipc::SentMessage;
 
namespace core
{
    HandlerType handler0_pingpong = [](TCPSocket& client, ReceivedStreamMessage& request, HandlerInfrastructure& infra)
    {
        auto msg = request[0].pData; 
        std::string send_msg{msg};
        SentMessage response;
        response.SetCommunicationCode(0);
        response.Add(send_msg.c_str(), send_msg.size() + 1);
        client.SendMessage(response);
    };

} // namespace core

_END_EXPRESS_NAMESPACE_