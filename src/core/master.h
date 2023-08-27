#ifndef _EXPRESS_CORE_MASTER_H
#define _EXPRESS_CORE_MASTER_H

#include <string>
#include <atomic>

#include "worker_pool.h"
#include "utility/macro.h"
#include "utility/ipc/tcp_server.h"


_START_EXPRESS_NAMESPACE_

namespace core
{
    class Master
    {
        public:
            Master(std::string port, int nworkers, int heart_beat_rate, int db_workers_channel);
            Master(const Master&) = delete;
            ~Master();

            /**
             * @note Run asynchronously. Thread-safety is not guaranteed for multi-masters.
            */
            void Run();
            void Stop();

        private:
            WorkerPool* pWorkerPool_;
            utility::ipc::TCPServer* pServer_;
            std::atomic_bool isRunning_;
    };
} // namespace core


_END_EXPRESS_NAMESPACE_


#endif