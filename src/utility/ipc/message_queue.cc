#include "message_queue.h"
#include "utility/misc.h"
#include <string>
#include <cstring>
#include <stdexcept>
#include <mqueue.h>
#include <signal.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <mqueue.h>

_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    using namespace misc;

    MessageQueue::MessageQueue(const std::string &name, long maxmsg, long msgsize)
        : msgsize_(msgsize), maxmsg_(maxmsg), closed_(false)
    {
        name_.push_back('/');
        name_.append(name);
        mq_attr attr;

        memset(&attr, 0, sizeof(mq_attr));
        attr.mq_flags = 0;
        attr.mq_maxmsg = maxmsg;
        attr.mq_msgsize = msgsize;
        mqdes_ = mq_open(name_.c_str(), O_CREAT | O_RDWR,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);

        if (mqdes_ < 0)
            throw std::runtime_error(
                utility::misc::FormatString("MessageQueue constructor: %s", 128, strerror(errno)));
    }

    MessageQueue::~MessageQueue()
    {
        if (!closed_)
            Close();
    }

    std::vector<std::pair<std::string, std::string>>
    MessageQueue::Limitation() const noexcept
    {
        using namespace std;

        vector<pair<string, string>> res;
        rlimit limit;

        getrlimit(RLIMIT_MSGQUEUE, &limit);
        auto open_max = sysconf(_SC_MQ_OPEN_MAX);
        auto prio_max = sysconf(_SC_MQ_PRIO_MAX);

        auto rlimit_mq = FormatString("cur=%d, max=%d", 32, limit.rlim_cur, limit.rlim_max);

        res.emplace_back("MQ_OPEN_MAX", to_string(open_max));
        res.emplace_back("MQ_PRIO_MAX", to_string(prio_max));
        res.emplace_back("RLIMIT_MSGQUEUE", rlimit_mq);

        return res;
    }

    void MessageQueue::Send(const Message &msg, uint priority)
    {
        auto msg_ptr = msg.Rawdata();
        if (mq_send(mqdes_, msg_ptr.get(), msg.Size(), priority) < 0)
            throw std::runtime_error(FormatString("MessageQueue-send: %s", 128,
                                                  strerror(errno)));
    }

    std::unique_ptr<MessageQueue::ReceiveReturnType> MessageQueue::Receive()
    {
        using namespace std;

        uint prio;
        unique_ptr<char[]> buf(new char[msgsize_]);
        ssize_t recv_size;
        if ((recv_size = mq_receive(mqdes_, buf.get(), msgsize_, &prio)) < 0)
            throw runtime_error(FormatString("MessageQueue-reiceive: %s", 128,
                                             strerror(errno)));

        unique_ptr<ReceiveReturnType> res =
            make_unique<ReceiveReturnType>(static_cast<const void *>(buf.get()), prio);
        return res;
    }

    void MessageQueue::SetAttribute(Attribute attr) noexcept
    {
        mq_attr attr_buf;
        switch (attr)
        {
        case Attribute::kDefault:
            attr_buf.mq_flags = 0;
            break;
        case Attribute::kNonblock:
            attr_buf.mq_flags = O_NONBLOCK;
            break;
        }
        memset(&attr_buf, 0, sizeof(attr_buf));

        mq_setattr(mqdes_, &attr_buf, nullptr);
    }

    MessageQueue::Attribute MessageQueue::GetAttribute() const noexcept
    {
        mq_attr buf;
        mq_getattr(mqdes_, &buf);

        switch (buf.mq_flags)
        {
        case 0:
            return Attribute::kDefault;
        case O_NONBLOCK:
            return Attribute::kNonblock;
        }

        // This statement will never be executed in normal case.
        return Attribute::kDefault;
    }

    void MessageQueue::CancelNotify() const noexcept
    {
        mq_notify(mqdes_, nullptr);
    }

    void MessageQueue::SetSignalNotify(int signo)
    {
        struct sigevent sigev;

        memset(&sigev, 0, sizeof(sigev));
        sigev.sigev_notify = SIGEV_SIGNAL;
        sigev.sigev_signo = signo;
        mq_notify(mqdes_, &sigev);
    }

    void MessageQueue::SetThreadNotify(void (*function)(sigval_t), sigval_t arg, pthread_attr_t *attribute)
    {
        struct sigevent sigev;

        memset(&sigev, 0, sizeof(sigev));
        sigev.sigev_notify = SIGEV_THREAD;
        sigev.sigev_value = arg;
        sigev._sigev_un._sigev_thread._attribute = attribute;
        sigev._sigev_un._sigev_thread._function = function;
        mq_notify(mqdes_, &sigev);
    }
}

_END_EXPRESS_NAMESPACE_