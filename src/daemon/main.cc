#include "daemon.h"
#include "init.h"
#include "config.h"

using namespace express::daemon;

int main(int argc, char* argv[])
{
    BecomeDaemon();
    FlagRunning();

    ParseArgs(argc, argv);
    InitConfig(); 

    InitDataBase();

    Run();

    return 0;
}
