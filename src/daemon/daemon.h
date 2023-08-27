/**
 * @file daemon.h
 * @author Hoo
 * @brief Provide the functions that daemon needs for itself.
*/
#ifndef _EXPRESS_DAEMON_DAEMON_H
#define _EXPRESS_DAEMON_DAEMON_H

#include <sys/types.h>
 
#include "daemon/component_pool.h"
#include "utility/macro.h"
#include "utility/os/local_process.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{  
    extern void BecomeDaemon();  
    /**
     * @brief Initialize the local variables of daemon.
    */
    extern void Prepare();
    extern void Run();
}

_END_EXPRESS_NAMESPACE_

#endif