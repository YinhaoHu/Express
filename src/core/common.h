/**
 * @file common.h
 * @brief Define a series of variables that can be used between multi 
 * components.
*/
#ifndef _EXPRESS_CORE_COMMON_H
#define _EXPRESS_CORE_COMMON_H

#include <signal.h>

#include <string_view>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace core
{
    inline constexpr const int kTermianteSignal = SIGTERM;
    inline constexpr const int kWorkerTerminateSignal = SIGTERM;
#ifdef _EXPRESS_DEBUG
    const std::string_view worker_exe_name("./bin/express-worker");
#else 
    const std::string_view worker_exe_name("/usr/sbin/express-worker");
#endif 
}

_END_EXPRESS_NAMESPACE_


#endif