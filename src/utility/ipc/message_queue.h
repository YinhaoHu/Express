#ifndef _EXPRESS_UTILITY_IPC_MESSAGE_QUEUE_H
#define __EXPRESS_UTILITY_IPC_MESSAGE_QUEUE_H

#include "utility/macro.h"
#include "message.h"
#include <mqueue.h>
#include <signal.h>
#include <vector>
#include <utility>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace ipc
    {
        /**
         * @brief A message queue class which encapsulates the POSIX message queue library.
         */
        class MessageQueue
        {
        public:
            enum class Attribute
            {
                kDefault,
                kNonblock
            };
            struct ReceiveReturnType
            {
                Message message;
                uint priority;
            };

        public:
            /**
             * @throw std::runtime_error will be thrown depending on the system library
             * implementation for message queue.
             */
            MessageQueue(const std::string &name, long maxmsg, long msgsize);

            MessageQueue(const MessageQueue &other) = delete;
            MessageQueue &operator=(const MessageQueue &rhs) = delete;

            ~MessageQueue();

            void Close() noexcept
            {
                mq_close(mqdes_);
                closed_ = true;
            }

            void Unlink() noexcept
            {
                mq_unlink(name_.c_str());
            }

            /**
             * @throw std::runtime_error will be thrown. The specific information
             * can be known in the chosen message queue manual.
             */
            void Send(const Message &msg, uint priority);

            /**
             * @throw std::runtime_error will be thrown. The specific information
             * can be known in the chosen message queue manual.
             */
            std::unique_ptr<ReceiveReturnType> Receive();

            /**
             * @return The current number of messages in queue.
             */
            size_t Size() const noexcept
            {
                mq_attr attr;
                mq_getattr(mqdes_, &attr);
                return attr.mq_curmsgs;
            }

            /**
             * @return A std::vector which contains limitations. Each element of the vector
             * is a std::pair whose first is the limitation key and second is the value.
             */
            std::vector<std::pair<std::string, std::string>> Limitation() const noexcept;

            void SetAttribute(Attribute attr) noexcept;

            Attribute GetAttribute() const noexcept;

            void CancelNotify() const noexcept;

            void SetSignalNotify(int signo);

            void SetThreadNotify(void (*function)(sigval_t), sigval_t arg, pthread_attr_t *attribute);

        private:
            bool closed_;
            long msgsize_, maxmsg_;
            mqd_t mqdes_;
            std::string name_;
        };

    }
}

_END_EXPRESS_NAMESPACE_

#endif