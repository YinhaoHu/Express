/**
 * @file component_pool.h
 * @author Hoo
 * @brief Define the ComponentPool class.
*/
#ifndef _EXPRESS_DAEMON_COMPONENT_POOL_H
#define _EXPRESS_DAEMON_COMPONENT_POOL_H

#include <unordered_map>

#include <utility/macro.h>
#include <utility/os/local_process.h>

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
         * @note `kCount` is used to get the size of this enumeration. It MUST be
         * in the last position and DON'T set the value manually.
         */
        enum class Component : unsigned int
        {
            kDataBase,
            kCore,
            kMonitor,
            kLog,
            kMail,
            kCount
        };

    public:
        ComponentPool();
        ComponentPool(const ComponentPool &) = delete;
        ~ComponentPool();

        /**
         * @brief Start the specified component.
         * @throw system_error will be thrown if system call set errno.
         */
        template <typename... ArgsType>
        void StartOne(Component component, const char *program, ArgsType... args);

        utility::os::LocalProcess &operator[](Component component);
        utility::os::LocalProcess *Find(pid_t pid);
        
        /**
         * @brief Tell all components to exits.
        */
        void Stop();

    private:
        std::unordered_map<Component, utility::os::LocalProcess *> components_;
        std::unordered_map<Component, std::string_view> components_alias_;
    };

    template <typename... ArgsType>
    void ComponentPool::StartOne(Component component, const char *program, ArgsType... args)
    {
        using utility::os::LocalProcess;

        components_[component] = new LocalProcess;

        components_[component]->Start(program, std::forward<ArgsType>(args)...);
        components_[component]->SetAlias(components_alias_[component].data());
    }

    extern ComponentPool* pComponentPool;
    void InitComponentPool();
}

_END_EXPRESS_NAMESPACE_

#endif