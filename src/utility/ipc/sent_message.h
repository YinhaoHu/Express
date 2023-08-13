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
         * A message class that encapsulate all data to send in a consecutive memory block.
         *
         * @note This message should be sent before all data from user is deleted.
         * The object has no ownership of the memory from Add() called by the user.
         *
         * @example
         *  string arg1("Hello.");
         *
         *  uint32_t comm_code = 32993;
         *
         *  SentMessage msg;
         *
         *  msg.SetCommunicationCode(comm_code);
         *
         *  msg.Add(arg1.c_str(), arg1.size() + 1);
         *
         *  IPC_channel.send(msg);
         */
        class SentMessage : public AbstractMessage
        { 
        private:
            std::shared_ptr<std::vector<Field>> spData;

        public:
            SentMessage() : spData(new std::vector<Field>)
            {
                spData->emplace_back(reinterpret_cast<const char*>(spHeader_.get()), spHeader_->Size());
            };
            ~SentMessage() = default;

            void Add(const char *pData, size_t size)
            {
                spHeader_->body_size += size + sizeof(size_t);
                spHeader_->num_of_fields++;

                spData->emplace_back(pData, size);
            }

            void SetCommunicationCode(uint32_t comm_code)
            {
                spHeader_->communication_code = comm_code;
            }

            /**
             * @note Used for stream IPC channels.
            */
            std::shared_ptr<std::vector<Field>> StreamData()const
            {
                return spData;
            }

            /**
             * @note Used for bounded IPC channels.
            */
            std::unique_ptr<char[]> MessageData() const
            {
                std::unique_ptr<char[]> res(new char[this->Size()]);

                auto p = res.get();
                size_t offset = 0;

                memcpy(p, spHeader_.get(), spHeader_->Size());
                offset += spHeader_->Size();
                for (size_t i = 0; i < spHeader_->num_of_fields; ++i)
                {
                    auto &item = spData->at(i+1);

                    memcpy(p + offset, &item.size, sizeof(item.size));
                    offset += sizeof(item.size);
                    memcpy(p + offset, item.pData, item.size);
                    offset += item.size;
                }

                return res;
            }
        };

    }

}
_END_EXPRESS_NAMESPACE_

#endif