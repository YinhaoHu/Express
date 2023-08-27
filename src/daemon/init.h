/**
 * @file init.h
 * @author Hoo.
 * @brief Provide a series of functions for initializing components.
*/
#ifndef _EXPRESS_DAEMON_INIT_H
#define _EXPRESS_DAEMON_INIT_H

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    // TODO: Use Local Process to update daemon init_xxx is also a consideration.
    extern void InitDataBase();
    extern void InitCore();
}

_END_EXPRESS_NAMESPACE_

#endif