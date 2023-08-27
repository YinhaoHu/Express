#include "reactor.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
// TODO : Delete this after debuging
#include <sys/socket.h>

#include <memory>
#include <thread>

#include "handlers/handler.h"
#include "utility/ipc/sent_message.h"
#include "utility/ipc/received_stream_message.h"
#include "utility/ipc/tcp_socket.h"
#include "utility/misc.h"

_START_EXPRESS_NAMESPACE_

using utility::ipc::ReceivedStreamMessage;
using utility::ipc::SentMessage;
using utility::ipc::TCPSocket;

namespace core
{
    Reactor::Reactor(int db_channel)
        : dbChannel_(db_channel), epfd_(0), isRunning_(true)
    {
        if ((epfd_ = epoll_create1(0)) < 0)
            utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("Reactor::Constructor"));

        std::thread react_thread([this]()
                                 {
            // TODO : `max_events` and `timeout` should be set in config file in the future
            InitHandler(this->dbChannel_);

            constexpr uint max_evnets = 1024, timeout_msec = -1;
            epoll_event *events = new epoll_event[max_evnets];

            int ready_num;
            while (this->isRunning_.load() == true)
            { 
                if ((ready_num = epoll_wait(epfd_, events, max_evnets, timeout_msec)) > 0)
                {
                    for (int i = 0; i < ready_num; ++i)
                    { 
                        TCPSocket client_socket(events[i].data.fd);
                        if (client_socket.IsPeerShutdown())
                        {
                            epoll_ctl(this->epfd_, EPOLL_CTL_DEL, client_socket.GetNative(), &(events[i]));
                            client_socket.Close();
                            continue;
                        }

                        try
                        {
                            auto request = client_socket.ReceiveMessage();
                            Handle(client_socket, *request);
                        }
                        catch (const std::exception &e)
                        {
                            // TODO : Log here
                            _EXPRESS_DEBUG_INSTRUCTION(syslog(LOG_DEBUG, "handler error: %s", e.what());)
                        }
                    }
            }
 
            }
            delete[] events; });
        react_thread.detach();
    }

    Reactor::~Reactor()
    {
        isRunning_.store(false);
        close(epfd_);
    }

    void Reactor::Add(int connfd)
    {
        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = connfd;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, connfd, &event);
    }
} // namespace core

_END_EXPRESS_NAMESPACE_