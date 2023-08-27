#include "init.h"

#include <unistd.h>
#include <syslog.h>

#include <string>
#include <cstring>

#include "daemon/param.h"
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
    static void InitDBAndWorkersChannel();

    void InitDataBase()
    {
        using Compo = ComponentPool::Component;

        InitDBAndWorkersChannel();
        try
        {
            // TODO: Moidfy "log-mq" here after log component is done.
            pComponentPool->StartOne(Compo::kDataBase, database_exe_name.data(), "express-database",
                                     pConfig->GetValue(Config::Key::kDataBaseDirName).c_str(),
                                     "log-mq",
                                     std::to_string(db_workers_channel.first).c_str(),
                                     pConfig->GetValue(Config::Key::kDataBaseNThreads).c_str());

            syslog(LOG_INFO, "%s (%d) was started successfully.",
                       (*pComponentPool)[Compo::kDataBase].GetAlias().data(),
                       (*pComponentPool)[Compo::kDataBase].GetID());
        }
        catch (const std::exception &e)
        {
            syslog(LOG_ERR, "InitCore(): %s", e.what());
            exit(EXIT_FAILURE);
        }
    }

    void InitCore()
    {
        using Compo = ComponentPool::Component;
        InitDBAndWorkersChannel();

        try
        {
            pComponentPool->StartOne(Compo::kCore, master_exe_name.data(), "express-core",
                                     std::to_string(db_workers_channel.second).c_str(),
                                     pConfig->GetValue(Config::Key::kCoreNWorkers).c_str(),
                                     pConfig->GetValue(Config::Key::kCorePort).c_str(),
                                     pConfig->GetValue(Config::Key::kCoreWorkerHeartBeatRate).c_str());

            syslog(LOG_INFO, "%s (%d) was started successfully.",
                   (*pComponentPool)[Compo::kCore].GetAlias().data(),
                   (*pComponentPool)[Compo::kCore].GetID());
        }
        catch (const std::exception &e)
        {
            syslog(LOG_ERR, "InitCore(): %s", e.what());
            exit(EXIT_FAILURE);
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
}

_END_EXPRESS_NAMESPACE_