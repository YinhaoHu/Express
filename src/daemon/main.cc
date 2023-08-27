#include "daemon.h"
#include "init.h"
#include "config.h"
#include "lockfile.h"

int main(int argc, char *argv[])
{
    using namespace express::daemon;

    BecomeDaemon();
    Prepare();

    InitConfig();
    
    InitDataBase();
    InitCore();

    Run();

    return 0;
}
