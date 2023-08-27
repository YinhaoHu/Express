#ifndef _EXPRESS_CORE_REACTOR_H
#define _EXPRESS_CORE_REACTOR_H

#include <mutex>
#include <queue>
#include <atomic>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_


namespace core
{
    class Reactor
    {
        public: 
            Reactor(int db_channel);
            Reactor(const Reactor&) = delete;
            ~Reactor();

            void Add(int connfd); 

        private:
            // TODO : This field should be set in the config file.
            const unsigned int autoCloseSec_ = 180;
            int dbChannel_, epfd_;
            std::atomic_bool isRunning_; 
    };
} // namespace core

_END_EXPRESS_NAMESPACE_


#endif