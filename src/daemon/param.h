/**
 * @file common.h
 * @author Hoo
 * @brief Specify the daemon's variables and parameters.
 */
#ifndef _EXPRESS_DAEMON_COMMON_H
#define _EXPRESS_DAEMON_COMMON_H

#include <signal.h>

#include <string_view>

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace daemon
{
        inline constexpr const int kTerminateSignal = SIGTERM;
        constexpr const std::string_view lockfile_name("/var/run/express-server.pid");
        constexpr const std::string_view config_file_name("/etc/express-server.conf");
        constexpr const std::string_view program_name("expressd");

#ifdef _EXPRESS_DEBUG
        constexpr const std::string_view working_dir(".");
        constexpr const std::string_view database_exe_name("./bin/express-db");
        constexpr const std::string_view master_exe_name("./bin/express-master");
        
#else
        constexpr const std::string_view working_dir("/");
        constexpr const std::string_view database_exe_name("/usr/sbin/express-db");
        constexpr const std::string_view master_exe_name("/usr/sbin/express-master");
#endif
}

_END_EXPRESS_NAMESPACE_

#endif