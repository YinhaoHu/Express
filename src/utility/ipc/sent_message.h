#ifndef _EXPRESS_UTILITY_IPC_SENT_MESSAGE_H
#define _EXPRESS_UTILITY_IPC_SENT_MESSAGE_H

#include "utility/ipc/abstract_message.h"
#include "utility/macro.h"
#include <memory>
#include <vector>
#include <stdexcept>

#ifdef __linux
#include <sys/uio.h>
#else 
#error "Provide the implementation of SentMessage::Data() on Windows."
#endif

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /** 
         * @example SentMessage
         * @brief The basic usage of this class.
         * @code
         *  string arg1("Hello.");
         *  uint32_t comm_code = 32993;
         *  SentMessage msg;
         *  msg.SetCommunicationCode(comm_code);
         *  msg.Add(arg1.c_str(), arg1.size() + 1);
         *  IPC_channel.send(msg);
         * @endcode
         */

        /**
         * @brief A message class that encapsulate all data to send in a consecutive 
         * memory block.
         *
         * @note This message should be sent before all data from user is deleted.
         * The object has no ownership of the memory from Add() called by the user.
         */
        class SentMessage : public AbstractMessage
        { 
        private:
            std::shared_ptr<std::vector<Field>> spData;

        public:
            SentMessage() ;
            SentMessage(SentMessage& ) = delete;
            ~SentMessage();

            void Add(const void *pData, size_t size);

            void SetCommunicationCode(uint32_t comm_code);

            /**
             * @note Used for stream IPC channels.
             * @warning It is a typical error that forget to send Field.size before 
             * Field.pData. To correctly use this function for sending Message,
             * you can learn how TCPClient::Send() do this.
            */
            std::shared_ptr<std::vector<Field>> StreamData()const;

            /**
             * @note Used for bounded IPC channels.
            */
            std::unique_ptr<char[]> MessageData() const;};

    }

}
_END_EXPRESS_NAMESPACE_

#endif