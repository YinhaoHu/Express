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

    public:
        LockFile(const char *filename);
        LockFile(const LockFile &) = delete;
        ~LockFile() ;

        void Lock();
        void Unlock();

        void SetRunningState(RunningState state);
        RunningState GetRunningState();

    private:
        int fd_;

        const struct
        {
            off_t pos = 0;
            size_t size = 1;
        } kRunningStateInfo;
    };

    extern LockFile* pLockFile;

    extern void InitLockFile();
}

_END_EXPRESS_NAMESPACE_

#endif