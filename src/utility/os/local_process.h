#ifndef _EXPRESS_UTILITY_OS_LOCAL_PROCESS_H
#define _EXPRESS_UTILITY_OS_LOCAL_PROCESS_H

#include <unistd.h>

#include <string>
#include <variant>
#include <string_view>

#include "utility/macro.h"
#include "utility/misc.h"

_START_EXPRESS_NAMESPACE_

namespace utility::os
{
    /**
     * @brief A process class which supports create a local host process and run program,
     * get the related status about it.
     */
    class LocalProcess
    {
    public:
        enum class Status
        {
            kNotRunning,
            kRunning,
            kStopped
        };

    public:
        LocalProcess()
            : pid_(0), alias_("")
        {
        }
        LocalProcess(const LocalProcess &) = delete;
        ~LocalProcess() = default;

        /**
         * @param program The path name of the program to be started.
         * @param args The argument list without nullptr as termination.
         * @throw std::system_error will be thrown if system call sets errno.
         */
        template <typename... ArgsType>
        void Start(const char *program, ArgsType... args)
        {
            pid_ = fork();

            if (pid_ < 0)
                utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO(" Start(): fork()"));
            else if (pid_ == 0)
            {
                if (execl(program, std::forward<ArgsType>(args)..., nullptr) < 0)
                    exit(EXIT_FAILURE);
            }
        }

        void SetAlias(const char *name)
        {
            alias_ = name;
        }

        inline pid_t GetID() const noexcept
        {
            return pid_;
        }

        inline const std::string_view &GetAlias() const noexcept
        {
            return alias_;
        }

        Status GetStatus() const noexcept
        {
            if (pid_ == 0)
                return Status::kNotRunning;
            else
                return (kill(pid_, 0) == 0) ? Status::kRunning : Status::kStopped;
        }

        void SendSignal(int signo)
        {
            kill(pid_, signo);
        }

    protected:
        pid_t pid_;
        std::string_view alias_;
    };

}
_END_EXPRESS_NAMESPACE_

#endif