#ifndef _EXPRESS_CORE_WORKER_POOL_H
#define _EXPRESS_CORE_WORKER_POOL_H

#include <list>

#include "worker.h"
#include "common.h"
#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace core
{
    class WorkerPool
    {
    public:
        using ListType = std::list<Worker>;

    public:
        WorkerPool(int nworkers, int heart_beat, int db_channel);
        WorkerPool(const WorkerPool &) = delete;
        ~WorkerPool();

        void Add();
        void Remove(ListType::const_iterator pos); 

        ListType &Get();

    private:
        ListType *pWorkers_;
        int heart_beat_rate_;
        int db_channel_;
    };

    inline WorkerPool::WorkerPool(int nworkers, int heart_beat, int db_channel)
        : pWorkers_(new ListType), heart_beat_rate_(heart_beat), db_channel_(db_channel)
    {
        for (int i = 0; i < nworkers; ++i)
            Add();
    }

    inline WorkerPool::~WorkerPool() 
    { 
        for(auto& worker : *pWorkers_)
            worker.SendSignal(kWorkerTerminateSignal);
        delete pWorkers_; 
    }

    inline void WorkerPool::Add()
    {
        pWorkers_->emplace_back(heart_beat_rate_, db_channel_);
    }

    inline void WorkerPool::Remove(ListType::const_iterator pos)
    {
        pWorkers_->erase(pos);
    }
 

    inline WorkerPool::ListType &WorkerPool::Get()
    {
        return *pWorkers_;
    }

} // namespace core

_END_EXPRESS_NAMESPACE_

#endif