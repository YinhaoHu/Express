#ifndef _EXPRESS_UTILITY_CONCURRENCY_THREADPOOL_H
#define _EXPRESS_UTILITY_CONCURRENCY_THREADPOOL_H

#include "utility/macro.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <type_traits>

_START_EXPRESS_NAMESPACE_

namespace utility::concurrency
{
    class ThreadPool
    {
    private:
        std::vector<std::thread> workers_;
        std::condition_variable tasks_cv_;
        std::mutex tasks_mutex_;
        std::queue<std::function<void()>> tasks_;
        bool stopped_;

    public:
        ThreadPool(size_t nthreads);

        ~ThreadPool();

        template <typename F, typename... Args>
        auto Add(F &&task, Args &&...args);
    };

    inline ThreadPool::ThreadPool(size_t nthreads)
        : stopped_(false)
    {
        for (size_t i = 0; i < nthreads; ++i)
        {
            workers_.emplace_back(
                [this]()
                {
                    std::function<void()> mytask;

                    for (;;)
                    {
                        {
                            std::unique_lock<std::mutex> lock(this->tasks_mutex_);
                            this->tasks_cv_.wait(lock, [&]()
                                                 { return !this->tasks_.empty() || this->stopped_; });

                            if (this->stopped_ && this->tasks_.empty())
                                return;
                            mytask = std::move(this->tasks_.front());
                            this->tasks_.pop();
                        }
                        mytask();
                    }
                });
        }
    }

    inline ThreadPool::~ThreadPool()
    {
        {
            std::scoped_lock<std::mutex> lock(tasks_mutex_);
            stopped_ = true;
        }
        tasks_cv_.notify_all();
        for (auto &worker : workers_)
            worker.join();
    }

    template <typename F, typename... Args>
    auto ThreadPool::Add(F &&task, Args &&...args)
    {
        using namespace std;
        using return_value_type = invoke_result_t<F, Args...>;

        auto async_task = make_shared<packaged_task<return_value_type()>>
        (packaged_task{
            [&]()
            {return task(forward<Args>(args)...);}
        }
        );

        {
            scoped_lock<std::mutex> lock(tasks_mutex_);
            if (stopped_)
                throw runtime_error("Thread pool stopped.");
            tasks_.emplace([async_task]()
                           { (*async_task)(); });
        }
        tasks_cv_.notify_one();
        return async_task->get_future();
    }
};

_END_EXPRESS_NAMESPACE_

#endif