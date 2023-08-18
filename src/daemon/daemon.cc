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
#include <cstring>

#include "daemon/config.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    ComponentPool::ComponentPool() : pid_array_(new pid_t[static_cast<size_t>(Component::kCount)]){};

    ComponentPool::~ComponentPool() { delete[] pid_array_; }

    void ComponentPool::SetPID(Component whose, pid_t pid) noexcept
    {
        pid_array_[static_cast<uint>(whose)] = pid;
    }

    pid_t ComponentPool::GetPID(Component whose) const noexcept
    {
        return pid_array_[static_cast<uint>(whose)];
    }

    ComponentPool::Component ComponentPool::GetComponent(pid_t pid) const noexcept
    {
        uint i;

        for (i = 0; (pid != pid_array_[i]) && i < static_cast<uint>(Component::kCount); ++i)
            ;

        return static_cast<Component>(i);
    }

    static constexpr struct
    {
        char state_closed = 0;
        char state_running = 1;
        off_t pos = 0;
        size_t size = sizeof(char);
    } kLockFileRunningState;

    ComponentPool *pComponentPool;

    static void ComponentGuard();

    static void ErrorQuit(const char *info = nullptr, bool log = false);

    static void FlagClosed();

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
        if (chdir("/") < 0)
            ErrorQuit("can not change directory to /.");

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

        openlog("Express-server", LOG_CONS, LOG_DAEMON);
        if (fd0 != 0 || fd1 != 1 || fd2 != 2)
        {
            syslog(LOG_ERR, "unexpected file descriptors: %d %d %d", fd0, fd1, fd2);
            exit(EXIT_FAILURE);
        }
    }

    void FlagRunning()
    {
        int fd = open(lock_file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        struct stat stat_buf;

        if (fd < 0)
            ErrorQuit("open lock file", true);

        if (flock(fd, LOCK_EX) < 0)
            ErrorQuit("lock file", true);

        memset(&stat_buf, 0, sizeof(stat_buf));
        fstat(fd, &stat_buf);
        if (stat_buf.st_size == 0)
            ftruncate(fd, sizeof(char));
        else
        {
            char running_state = 0;

            pread(fd, &running_state, sizeof(running_state), kLockFileRunningState.pos);
            if (running_state == kLockFileRunningState.state_running)
            {
                syslog(LOG_ERR, "daemon is already running");
                exit(EXIT_FAILURE);
            }
            else
            {
                pwrite(fd, &kLockFileRunningState.state_running, kLockFileRunningState.size,
                       kLockFileRunningState.pos);
                atexit(FlagClosed);
            }
        }
        if (flock(fd, LOCK_UN) < 0)
            ErrorQuit("unlock file", true);

        close(fd);

        pComponentPool = new daemon::ComponentPool{};

        std::thread component_guard_thread(ComponentGuard);
        component_guard_thread.detach();
    }

    void Run()
    {
        // TODO Decrease the priority of daemon when release.
        if (nice(19) < 0)
            ErrorQuit("schedule priority decreasement failure", true);

        syslog(LOG_INFO, "Daemon(%d) is running now.", getpid());
        while (1)
            ;
    }

    static void ComponentGuard()
    {
        _EXPRESS_DEBUG_INSTRUCTION(syslog(LOG_DEBUG, "component guard started successfully.");)

        int exit_code = 0;
        auto DoLog = [&](const char *which)
        {
            syslog(LOG_EMERG, "%s corrupted(%d) - from component guard", which, exit_code);
            exit(EXIT_FAILURE);
        };
        for (;;)
        {
            using compo = ComponentPool::Component;

            pid_t exit_pid = wait(&exit_code);

            switch (pComponentPool->GetComponent(exit_pid))
            {
            case compo::kDataBase:
                DoLog("database");
                break;
            case compo::kLog:
                /* code */
                break;
            case compo::kMail:
                /* code */
                break;
            case compo::kMaster:
                /* code */
                break;
            case compo::kMonitor:
                /* code */
                break;
            default:
                break;
            }
        }
    }

    static void ErrorQuit(const char *info, bool log)
    {
        if (log)
            syslog(LOG_ERR, "%s: %s", info, strerror(errno));
        else
            std::cerr << std::format("{}: {} ({})\n", daemon::prog_name, info, strerror(errno));

        exit(EXIT_FAILURE);
    }

    static void FlagClosed()
    {
        int fd = open(daemon::lock_file_name, O_WRONLY);
        if (fd < 0)
            ErrorQuit("open lock file", true);
        if (flock(fd, LOCK_EX) < 0)
            ErrorQuit("lock file lock", true);
        pwrite(fd, &kLockFileRunningState.state_closed,
               kLockFileRunningState.size, kLockFileRunningState.pos);
        if (flock(fd, LOCK_UN) < 0)
            ErrorQuit("unlock file", true);
        close(fd);
    }

} // namespace daemon

_END_EXPRESS_NAMESPACE_