#ifndef _EXPRESS_DAEMON_DAEMON_H
#define _EXPRESS_DAEMON_DAEMON_H

#include <sys/types.h>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    /**
     * @brief A component process controlling class.
     */
    class ComponentPool
    {
    public:
        /**
         * @note `kCount` is used to get the size of Key. It MUST be
         * in the last position and DON'T set the value manually.
         */
        enum class Component : unsigned int
        {
            kDataBase,
            kMaster,
            kMonitor,
            kLog,
            kMail,
            kCount
        };

    public:
        ComponentPool();
        ComponentPool(const ComponentPool &) = delete;
        ~ComponentPool();

        void SetPID(Component whose, pid_t pid) noexcept;
        pid_t GetPID(Component whose) const noexcept;
        Component GetComponent(pid_t pid) const noexcept;

    private:
        pid_t *pid_array_;
    };

    extern ComponentPool *pComponentPool;

    extern void BecomeDaemon(); 
    extern void Prepare();
    extern void Run();
}

_END_EXPRESS_NAMESPACE_

#endif