#include "daemon.h"

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <thread>
#include <iostream>
#include <format>
#include <filesystem>
#include <cstring>

#include "lockfile.h"
#include "daemon/component_pool.h"
#include "daemon/param.h"
#include "daemon/config.h"
#include "core/common.h"
#include "database/public.h"
#include "utility/ipc/signal.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    // Local global variables and functions 

    static void ErrorQuit(const char *info = nullptr, bool log = false);

    static void TerminateHandler(int);

    // Non-local   functions
    void BecomeDaemon()
    {
        if (getuid() != 0)
        {
            std::cerr << "Error: Express server should be run as a root\n";
            exit(EXIT_FAILURE);
        }

        //  File creation mode mask.
        umask(007);

        //  Become a session leader and lose control of tty.
        pid_t pid = fork();
        if (pid < 0)
            ErrorQuit("fork for becoming session leader.");
        else if (pid > 0)
            exit(EXIT_SUCCESS);

        setsid();

        // Ensure future opens won't allocate controlling terminal.
        struct sigaction act;

        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = SIG_IGN;
        if (sigaction(SIGHUP, &act, nullptr) < 0)
            ErrorQuit("can not ignore SIGHUP.");
        if ((pid = fork()) < 0)
            ErrorQuit("can not fork");
        else if (pid != 0)
            exit(EXIT_SUCCESS);

        // Change working directory.
        if (chdir(working_dir.data()) < 0)
            ErrorQuit("can not change working directory.");

        // Close all unnecssary file descriptors.
        rlimit nopenfd;

        if (getrlimit(RLIMIT_NOFILE, &nopenfd) < 0)
            ErrorQuit("can not get limit of RLIMT_NOFILE.");

        if (nopenfd.rlim_max == RLIM_INFINITY)
            nopenfd.rlim_max = 1024;
        for (int fd = 0; fd < static_cast<int>(nopenfd.rlim_max); ++fd)
            close(fd);

        // Forbid the interacting with tty.
        int fd0 = open("/dev/null", O_RDWR);
        int fd1 = dup(fd0);
        int fd2 = dup(fd0);

        openlog(program_name.data(), LOG_CONS, LOG_DAEMON);
        if (fd0 != 0 || fd1 != 1 || fd2 != 2)
        {
            syslog(LOG_ERR, "unexpected file descriptors: %d %d %d", fd0, fd1, fd2);
            exit(EXIT_FAILURE);
        }
    }

    void Prepare()
    {
        //  Ensure there is only one daemon running.
        InitLockFile();
        try
        {
            pLockFile->Lock();
            if (pLockFile->GetRunningState() != LockFile::RunningState::kNotRunning)
            {
                syslog(LOG_ERR, "Error: daemon has already been running.\n");
                exit(EXIT_FAILURE);
            }
            pLockFile->SetRunningState(LockFile::RunningState::kRunning);
            pLockFile->Unlock();
        }
        catch (const std::system_error &e)
        {
            syslog(LOG_ERR, "%s", e.what());
            exit(EXIT_FAILURE);
        }

        // Load component pool.
        InitComponentPool();

        // Load component guard thread.
        std::thread component_guard_thread([]()
                                           {
                int exit_code = 0;
        auto DoLog = [&](const char *which)
        {
            syslog(LOG_EMERG, "%s corrupted(%d) - from component guard", which, exit_code);
            raise(kTerminateSignal);
        };

        for (;;)
        {
            pid_t exit_pid = wait(&exit_code);
            auto pComp = pComponentPool->Find(exit_pid);

            if (pComp != nullptr)
                DoLog(pComp->GetAlias().data());
        } });
        component_guard_thread.detach();
    }

    void Run()
    {
        using utility::ipc::Signal;

        if (nice(19) < 0)
            ErrorQuit("nice value set failure", true);

        Signal(kTerminateSignal, TerminateHandler);

        int pending_sig;
        sigset_t event_sig_set;
        sigemptyset(&event_sig_set);

        syslog(LOG_INFO, "Daemon(%d) is running now.", getpid());
        for (;;)
        {
            sigwait(&event_sig_set, &pending_sig);
            switch (pending_sig)
            {
            // TODO: reserved to capture some signals.
            default:
                syslog(LOG_ERR, "Daemon received an unexpected signal.");
                break;
            }
        }
    }

    // Local  functions 

    static void ErrorQuit(const char *info, bool log)
    {
        if (log)
            syslog(LOG_ERR, "%s: %s", info, strerror(errno));
        else
            std::cerr << std::format("{}: {} ({})\n", program_name, info, strerror(errno));

        exit(EXIT_FAILURE);
    }

    static void TerminateHandler(int)
    {
        pComponentPool->Stop();

        pLockFile->SetRunningState(LockFile::RunningState::kNotRunning);
        syslog(LOG_INFO, "Daemon(%d) is closed.", getpid());
        exit(0);
    }
} // namespace daemon

_END_EXPRESS_NAMESPACE_