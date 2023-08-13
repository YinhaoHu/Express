#ifndef _EXPRESS_UTILITY_IPC_RECEIVED_STREAM_MESSAGE_H
#define _EXPRESS_UTILITY_IPC_RECEIVED_STREAM_MESSAGE_H

#include "utility/ipc/abstract_message.h"
#include "utility/macro.h"
#include <memory>
#include <vector>
#include <stdexcept>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /**
         * A message class that is used to receive messages from stream IPC channel.
         * 
         * @note All data in this class has the life time which is same as this class
         * object.
         * 
         * @example 
         *  std::shared_ptr<Header> spHeader;
         * 
         *  IPC_channel.receive(spHeader->GetDataHandler(),spHeader->Size());
         * 
         *  ReceivedStreamMessage msg(spHeader);
         * 
         *  IPC_channel.receive(msg.GetBodyHandler, spHeader->body_size);
         * 
         *  msg.ValidateBody();
         * 
         *  auto arg1_pointer = msg[0].pData;
         * 
         *  auto arg1_size = msg[0].size; 
         */
        class ReceivedStreamMessage : public AbstractMessage
        {
        private:
            bool body_validation_;
            std::shared_ptr<std::vector<Field>> spFields_;
            std::shared_ptr<char[]> spBody_;

        public:
            /**
             * Construct from IPC data whose ownership will not be be transferred
             * for the sake of convenience.
             */
            ReceivedStreamMessage(std::shared_ptr<Header> spheader)
                : AbstractMessage(spheader), body_validation_(false),
                  spFields_(new std::vector<Field>), spBody_(new char[spheader->body_size])
            { 
            }
            
            ReceivedStreamMessage(const ReceivedStreamMessage& other)
                :body_validation_(other.body_validation_), spFields_(other.spFields_),
                spBody_(other.spBody_)
            {

            }

            ~ReceivedStreamMessage() = default;

            /**
             * Used to receive data from IPC channel.
            */
            std::shared_ptr<char[]> GetBodyHandler()
            {
                return spBody_;
            }

            /**
             * Validate the body part and object will allow operator[].
            */
            void ValidateBody()
            {
                auto p = spBody_.get();
                for (size_t i = 0, offset = 0; i < spHeader_->num_of_fields; ++i)
                {
                    size_t data_size;

                    memcpy(&data_size, p + offset, sizeof(data_size));
                    offset += sizeof(data_size);
                    spFields_->emplace_back(p + offset, data_size);
                    offset += data_size;
                }
                body_validation_ = true;
            }

            const Field operator[](size_t id)
            {
                if (!body_validation_)
                    throw std::runtime_error("ReceivedStreamMessage::operator[]:Body is not valid yet.");
                if (id >= GetHeaderField(Header::Field::kNumOfFields))
                    throw std::out_of_range("ReceivedStreamMessage:: operator[] out of range.");
                else
                    return (*spFields_)[id];
            }
        };

    }

}
_END_EXPRESS_NAMESPACE_

#endif