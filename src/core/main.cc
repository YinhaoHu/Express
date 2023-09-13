/**
 * @file main.cc
 * @brief Start the one/several master(s).
*/
#include <sys/resource.h>
#include <syslog.h>

#include "core/common.h"
#include "core/master.h"
#include "core/config.h"
#include "utility/ipc/signal.h"

using express::core::Config;
using express::core::Master;
using express::utility::ipc::Signal;

static Master *pMaster;

static void Init(char *argv[]);
static void Run();
static void TerminateHandler(int);

/**
 * @note Explaination for `argv`
 * [1]  db_workers_channel  ->  int     :   The socket used to communicate with database.
 * [2]  nworkers            ->  int     :   The number of workers(processes).
 * [3]  port                ->  cstring :   The port from which the master will accept.
 * [4]  heartbeat rate      ->  int     :   Specify the worker sends heart beat to master every
 *                                          # seconds.
 */
int main(int argc, char *argv[])
{ 
    Init(argv);
    Run();

    return 0;
}

static void Init(char *argv[])
{
    // TODO: nofile limit should be considered to be set in config file.
    rlimit nofile_limit;
    getrlimit(RLIMIT_NOFILE, &nofile_limit);
    nofile_limit.rlim_cur = nofile_limit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &nofile_limit) < 0)
    {
        syslog(LOG_ERR, "error in set nofile. (from master)");
        exit(EXIT_FAILURE);
    }

    Config config(argv);

    pMaster = new Master(std::get<std::string>(config.GetConfig(Config::Key::kPort)),
                         std::get<int>(config.GetConfig(Config::Key::kNWorkers)),
                         std::get<int>(config.GetConfig(Config::Key::kHearBeatRate)),
                         std::get<int>(config.GetConfig(Config::Key::kDBWorkersChannel)));

    Signal(express::core::kTermianteSignal, TerminateHandler);
}

static void Run()
{
    pMaster->Run();

    int sig;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, express::core::kTermianteSignal);

    while (sigwait(&set, &sig) >= 0)
    {
        if (sig == express::core::kTermianteSignal)
        {
            delete pMaster;
            exit(EXIT_SUCCESS);
        }
    }
    // TODO: Log error.
}

static void TerminateHandler(int)
{
    delete pMaster;

    exit(EXIT_SUCCESS);
}