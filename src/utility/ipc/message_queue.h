#ifndef _EXPRESS_UTILITY_IPC_MESSAGE_QUEUE_H
#define __EXPRESS_UTILITY_IPC_MESSAGE_QUEUE_H

#include "utility/macro.h"
#include "utility/ipc/sent_message.h"
#include "utility/ipc/received_boundary_message.h"
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
                ReceivedBoundaryMessage message;
                uint priority;
            };

        public:
            /**
             * @throw std::system_error will be thrown depending on the system library
             * implementation for message queue.
             */
            MessageQueue(const std::string &name, long maxmsg, long msgsize);

            /**
             * @throw std::system_error will be thrown depending on the system library
             * implementation for message queue.
             */
            MessageQueue(const std::string &name);

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
            void Send(const SentMessage &msg, uint priority);

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
             * @return The max number of in-queue message allowed.
            */
            size_t MaxSize() const noexcept
            {
                mq_attr attr;
                mq_getattr(mqdes_, &attr);
                return attr.mq_maxmsg;
            }

            /**
             * @return The max size of one message allowed.
            */
            size_t MaxMessageLength() const noexcept
            {
                mq_attr attr;
                mq_getattr(mqdes_, &attr);
                return attr.mq_msgsize;
            }

            /**
             * @return A std::vector which contains limitations. Each element of the vector
             * is a std::pair whose first is the limitation key and second is the value.
             */
            std::vector<std::pair<std::string, std::string>> Limitation() const noexcept;

            void SetAttribute(Attribute attr) noexcept;

            Attribute GetAttribute() const noexcept;

            void CancelNotify() noexcept;

            void SetSignalNotify(int signo);

            void SetThreadNotify(void (*function)(sigval_t), sigval_t arg, pthread_attr_t *attribute);

        private:
            long msgsize_, maxmsg_;
            bool closed_;
            mqd_t mqdes_;
            std::string name_;
        };

    }
}

_END_EXPRESS_NAMESPACE_

#endif