/**
 * @file lockfile.h
 * @author Hoo
 * @brief Lock file is used for daemon to record some information like
 * running state and prevent from concurrent issues.
 */

#ifndef _EXPRESS_DAEMON_LOCKFILE_H
#define _EXPRESS_DAEMON_LOCKFILE_H

#include <sys/types.h>

#include <cstddef>
#include <string_view>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    class LockFile
    {
    public:
        enum class RunningState : char
        {
            kNotRunning,
            kRunning
        };

    protected:
        LockFile(std::string_view file_name);

    public:
        LockFile(const LockFile &) = delete;
        ~LockFile();

        void Lock();
        void Unlock();

        void SetRunningState(RunningState state);
        RunningState GetRunningState();

        static LockFile* Instance();

    private:
        static LockFile* instance_;

        int fd_;

        const struct
        {
            off_t pos = 0;
            size_t size = 1;
        } kRunningStateInfo;
    };
 
}

_END_EXPRESS_NAMESPACE_

#endif