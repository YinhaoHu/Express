#include "lockfile.h"

#include <unistd.h>
#include <sys/file.h>

#include <iostream>
#include <filesystem>
#include <string_view>

#include "daemon/param.h"
#include "utility/macro.h"
#include "utility/misc.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    LockFile* pLockFile;

    void InitLockFile()
    {
        try
        {
            pLockFile = new LockFile(daemon::lockfile_name.data());
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        } 
    }

    LockFile::LockFile(const char *filename)
    {
        if (!std::filesystem::exists(filename))
            fd_ = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        else
            fd_ = open(filename, O_RDWR);

        if (fd_ < 0)
            utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("LockFile::LockFile()"));
    }

    LockFile::~LockFile()
    {
        close(fd_);
    }

    void LockFile::Lock()
    {
        if (flock(fd_, LOCK_EX) < 0)
            utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("LockFile::Lock()"));
    }

    void LockFile::Unlock()
    {
        if (flock(fd_, LOCK_UN) < 0)
            utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("LockFile::Unlock()"));
    }

    void LockFile::SetRunningState(RunningState state)
    { 
        char buf = static_cast<char>(state);
        if (pwrite(fd_, &buf, kRunningStateInfo.size, kRunningStateInfo.pos) < 0)
            utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("LockFile::SetRunningState(): pwrite"));
    }

    LockFile::RunningState LockFile::GetRunningState()
    { 
        char buf;
        if (pread(fd_, &buf, kRunningStateInfo.size, kRunningStateInfo.pos) < 0)
            utility::misc::ThrowSystemError(SYSTEM_ERROR_INFO("LockFile::GetRunningState(): pwrite"));

        return static_cast<RunningState>(buf);
    }
}

_END_EXPRESS_NAMESPACE_
