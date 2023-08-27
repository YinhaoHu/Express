#ifndef _EXPRESS_UTILITY_IPC_TCP_SOCKET_H
#define _EXPRESS_UTILITY_IPC_TCP_SOCKET_H

#include "utility/macro.h"
#include "utility/data/rawdata.h"
#include "utility/ipc/basic_socket.h"
#include "utility/ipc/socket.h"
#include "utility/ipc/received_stream_message.h"
#include "utility/ipc/sent_message.h"

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /**
         * @brief A tcp socket wrapper class.
         */
        class TCPSocket : public BasicSocket
        {
        public:
            /**
             * @throw std::runtime_error will be thrown when system call set errno.
             */
            TCPSocket(InternetProtocol ip_version)
                : BasicSocket(SystemIPConstant(ip_version), SOCK_STREAM)
            {
            }

            TCPSocket(const TCPSocket& )= delete;

            TCPSocket(Socket sockfd)
                : BasicSocket(sockfd)
            {
            }

            ~TCPSocket() = default;

            void SendMessage(const SentMessage &msg)
            {
                const auto &spData = *msg.StreamData();

                Send(spData[0].pData, spData[0].size, MSG_MORE);

                size_t last_idx = spData.size() - 1;
                for (size_t i = 1; i < last_idx; ++i)
                {
                    Send(reinterpret_cast<const char *>(&(spData[i].size)),
                                 sizeof(spData[i].size), MSG_MORE);
                    Send(spData[i].pData, spData[i].size, MSG_MORE);
                }

                Send(reinterpret_cast<const char *>(&(spData[last_idx].size)),
                             sizeof(spData[last_idx].size), MSG_MORE);
                Send(spData[last_idx].pData, spData[last_idx].size, MSG_WAITALL);
            }

            /**
             * @note If peer shutdown this socket stream, this function call will block.
             * Use IsPeerShutdown to detect whether the peer is alive or not.
            */
            std::unique_ptr<ReceivedStreamMessage> ReceiveMessage()
            {
                using Header = ReceivedStreamMessage::Header;
                std::shared_ptr<Header> spHeader(new Header); 
                Receive(spHeader->GetData(), spHeader->Size(), MSG_WAITALL); 
                
                auto msg = std::make_unique<ReceivedStreamMessage>(spHeader); 
                Receive(msg->GetBodyHandler().get(), spHeader->body_size, MSG_WAITALL);
                msg->ValidateBody();
 
                return msg ;
            }
        
            /**
             * @note This function assumes that recv will not set errno.
            */
            bool IsPeerShutdown()
            {
                char buf[1];
                return (recv(GetNative(), buf, 1, MSG_PEEK) == 0);
            }
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif