#include "init.h"

#include <unistd.h>
#include <syslog.h>

#include <string>
#include <cstring>

#include "common.h"
#include "daemon/config.h"
#include "daemon/daemon.h"
#include "utility/macro.h"
#include "utility/ipc/unix_domain_socket.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    /**
     * @note `.first` is used for db and `.second` for workers.
    */
    static std::pair<Socket, Socket> db_workers_channel;
    static void ErrorQuit(const char *info = nullptr);
    static void InitDBAndWorkersChannel();

    void InitDataBase()
    {
        InitDBAndWorkersChannel();
        pid_t pid = fork();

        if (pid < 0)
            ErrorQuit("fork database process");
        else if (pid == 0)
        {
            if (execl(common::database_exe_name, "express-database",
                      pConfig->GetValue(Config::Key::kDataBaseDirName).c_str(),
                      "log-mq",
                      std::to_string(db_workers_channel.first).c_str(),
                      pConfig->GetValue(Config::Key::kDataBaseNThreads).c_str(),
                      nullptr) < 0)
                ErrorQuit("execute database");
            else 
                syslog(LOG_INFO, "database was loaded sucessfully.");
        }
        else 
        {
            pComponentPool->SetPID(ComponentPool::Component::kDataBase, pid);
        }
    }

    static void InitDBAndWorkersChannel()
    {
        static bool inited = false;
        if (!inited)
        {
            db_workers_channel = utility::ipc::UnixDomainSocket::SocketPair();
            inited = true;
        }
    }
    
    static void ErrorQuit(const char *info)
    {
        if (errno != 0)
            syslog(LOG_ERR, "%s: %s", info, strerror(errno));
        else
            syslog(LOG_ERR, "%s", info);

        exit(EXIT_FAILURE);
    }
}

_END_EXPRESS_NAMESPACE_