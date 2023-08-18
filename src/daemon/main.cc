#include "daemon.h"
#include "init.h"
#include "config.h"

using namespace express::daemon;

int main(int argc, char* argv[])
{
    ParseArgs(argc, argv); 
    BecomeDaemon(); 

    Prepare();
    InitConfig();  
    InitDataBase();

    Run();

    return 0;
}
