#ifndef _EXPRESS_COMMON_H
#define _EXPRESS_COMMON_H

#include "utility/macro.h"

_START_EXPRESS_NAMESPACE_

namespace common
{
    inline constexpr const char *database_exe_name =
#ifdef _EXPRESS_DEBUG
        "./bin/db-server";
#else
        "/usr/sbin/express-database";
#endif
} // namespace public

_END_EXPRESS_NAMESPACE_

#endif