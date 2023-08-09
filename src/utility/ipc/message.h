#ifndef _EXPRESS_UTILITY_IPC_MESSAGE_H
#define _EXPRESS_UTILITY_IPC_MESSAGE_H

#include "utility/macro.h"
#include <stddef.h>
#include <stdint.h>
#include <cstring>
#include <memory>
#include <vector>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /**
         * @brief A IPC(Local and Net) message class.
         *
         * @warning `MEMORY MANAGEMENT`
         * [Sender] needs to takes the responsibility for the data which is added into the message by calling Add().
         * [Reiceiver] takest he responsibility for all data including header and body which are added.
         * [This class] will return the data it manages by returning smart pointer.
         */
        class Message
        {
        public:
            enum class HeaderField
            {
                kCommunicationCode,
                kNumOfFields,
                KBodySize
            };

            struct Header
            {
                uint8_t num_of_fields;
                char reserved[11];
                uint32_t communication_code;
                uint64_t body_size;

                Header() : communication_code(0), num_of_fields(0),
                           body_size(0) {}

                Header(uint32_t comm_code) : communication_code(comm_code),
                                             num_of_fields(0), body_size(0) {}

                Header(const Header &other) : communication_code(other.communication_code),
                                              num_of_fields(other.num_of_fields),
                                              body_size(other.body_size) {}

                static constexpr size_t Size() noexcept
                {
                    return sizeof(Header);
                }
            };

            struct Field
            {
                const void *pointer;
                size_t size;
            };

        private:
            enum class DeleteMode
            {
                kOneByOne,
                kIntegrally,
            };
            DeleteMode delete_mode_;
            Header *header_;
            std::vector<Field> *fields_;

        public:
            /**
             * @note This constructor should be used for sending.
             */
            Message(uint32_t communication_code);
            /**
             * @note This constructor should be used for receiving in bytestream-based
             * IPC channel like socket or pipe.
             */
            Message(const Header &header);

            /**
             * @note This constructor is useful for known entire message data in
             * a consecutive memory block.You might use it in MessageQueue receive.
             */
            Message(const void *rawdata);

            Message(const Message &other) = delete;
            Message(Message &&other) = delete;
            Message &operator=(const Message &rhs) = delete;

            ~Message();

            /**
             * @brief Add data to the message. The data is shared from the user
             * for this class. This class does not manage the memory you share with it.
             */
            void Add(const void *buf, size_t size) noexcept;

            /**
             * @return A unique pointer to vector whose elements are message body fields.
             */
            std::unique_ptr<std::vector<Field>> Body() const noexcept;

            /**
             * @brief Get the specified field in the header part.
             */
            uint64_t GetHeaderField(HeaderField field) const noexcept;

            /**
             * @return The total size including header and body.
             */
            constexpr size_t Size() const noexcept
            {
                return header_->Size() + header_->body_size;
            }

            /**
             * @brief Conveniently reclaim the memory that you share with this class.
             * @note You might construct one Message object by consecutive memory, which
             * contains the header and the body parts. In other words, the memory was
             * obtained by new char[size]. It might be also a case that you share the
             * non-consecutive memory with this class, you might need to delete them one by one.
             * So, you can delete the data you shared with this class by calling this f
             * unction even you don't know how the data was allocated initially.
             */
            void Delete();

            /**
             * @return Unique poitner to a consecutive memory block including header and body.
             * @note This function should be used when you plan to send the data over IPC channel.
             */
            std::unique_ptr<char[]> Rawdata() const noexcept;

            /**
             * @brief Set the fields part according to the consecutive memory block.
             */
            void SetFields(const void *fields_rawdata) noexcept;
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif