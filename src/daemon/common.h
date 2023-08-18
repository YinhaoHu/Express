#ifndef _EXPRESS_DAEMON_COMMON_H
#define _EXPRESS_DAEMON_COMMON_H

#include <signal.h>

#include <string_view>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
    inline constexpr const int kTerminateSignal = SIGTERM;

#ifdef _EXPRESS_DEBUG
        constexpr const std::string_view working_dir(".");
        constexpr const char *database_exe_name = "./bin/db-server";
#else
        constexpr const std::string_view working_dir("/");
        constexpr const char *database_exe_name = "/usr/sbin/express-database";
#endif
}

_END_EXPRESS_NAMESPACE_

#endif