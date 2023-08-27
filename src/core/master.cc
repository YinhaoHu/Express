#include "master.h"

#include <syslog.h>

#include <thread>

#include "worker_pool.h"
#include "utility/ipc/tcp_server.h"

_START_EXPRESS_NAMESPACE_

using utility::ipc::InternetProtocol;
using utility::ipc::TCPServer;

namespace core
{
    Master::Master(std::string port, int nworkers, int heart_beat_rate, int db_workers_channel)
        : pWorkerPool_(new WorkerPool(nworkers, heart_beat_rate, db_workers_channel)),
          pServer_(new TCPServer(port.c_str(), InternetProtocol::kIPv4)),
          isRunning_(true)
    {
    }

    Master::~Master()
    {
        Stop();

        delete pWorkerPool_;
    }

    void Master::Run()
    {
        std::thread running_thread([this]()
                                   {
            auto& workers=this->pWorkerPool_->Get();
            auto worker_iter = workers.begin();

            while(this->isRunning_.load())
            {  
                try
                {
                   if(this->pServer_->WaitForConnection())
                {  
                    _EXPRESS_DEBUG_INSTRUCTION(syslog(LOG_DEBUG, "master received a connection.");)
                    // Schedule snippet using simple round-robin policy.
                    // TODO: Implement a more flexiable schedule policy based on 
                    // the cpu usage, memory usage, error rate and latency in the future.
                    if(worker_iter == workers.end())
                        worker_iter = workers.begin();

                    auto connection = this->pServer_->NextPending(); 
                    worker_iter->AddTask(connection->GetNative());
                    connection->Close();
 
                    ++worker_iter;
                }
                }
                catch(const std::exception& e)
                {
                    syslog(LOG_ERR, "master error: %s",e.what());
                }
                
                
            } });

        running_thread.detach();
    }

    void Master::Stop()
    {
        this->isRunning_.store(false);
    }
}

_END_EXPRESS_NAMESPACE_