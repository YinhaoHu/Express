#ifndef _EXPRESS_UTILITY_IPC_RECEIVED_BOUNDARY_MESSAGE_H
#define _EXPRESS_UTILITY_IPC_RECEIVED_BOUNDARY_MESSAGE_H

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
         * @example ReceivedBoundaryMessage
         * @brief The basic usage of this class.
         * @code
         *  auto ipc_data = IPC_channel.receive(); 
         *  ReceivedBoundaryMessage msg(std::move(ipc_data)); 
         *  auto arg1_pointer = msg[0].pData; 
         *  auto arg1_size = msg[0].size; 
         * @endcode
         */

        /**
         * @brief A message class that is used to receive messages from bounded IPC channel.
         * 
         * @note All data in this class has the life time which is same as this class
         * object. 
         */
        class ReceivedBoundaryMessage : public AbstractMessage
        {
        private:
            std::shared_ptr<std::vector<Field>> spFields_;
            std::shared_ptr<char[]> spFullMessage_;

        public:
            /**
             * Construct from IPC data whose ownership should be transferred.
             */
            ReceivedBoundaryMessage(std::unique_ptr<char[]> spFullMessage)
                : AbstractMessage(), spFields_(new std::vector<Field>),
                  spFullMessage_(std::move(spFullMessage))
            {
                (*spHeader_) = *reinterpret_cast<Header *>(spFullMessage_.get());

                for (size_t i = 0, offset = spHeader_->Size();
                     i < spHeader_->num_of_fields; ++i)
                {
                    size_t cursize = *reinterpret_cast<size_t *>(static_cast<char *>(spFullMessage_.get()) + offset);

                    offset += sizeof(size_t);

                    spFields_->emplace_back(
                        static_cast<char *>(spFullMessage_.get()) + offset, cursize);

                    offset += cursize;
                }
            }
            
            ReceivedBoundaryMessage(const ReceivedBoundaryMessage& other)
            : AbstractMessage(other),spFields_(other.spFields_), spFullMessage_(other.spFullMessage_) 
            {

            }

            ~ReceivedBoundaryMessage() = default;

            /**
             * Access the specified field.
             * @warning The data has the life time which is same as this class
             * object.
             */
            const Field &operator[](size_t id)
            {
                if (id >= GetHeaderField(Header::Field::kNumOfFields))
                    throw std::out_of_range("ReceivedStreamMessage: operator[] out of range.");
                else
                    return (*spFields_)[id];
            }
        };

    }

}
_END_EXPRESS_NAMESPACE_

#endif