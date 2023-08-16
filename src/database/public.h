/**
 * @file public.h
 * @note Declare some types that can be used in client and server
 * in the database system.
*/

#ifndef _EXPRESS_DATABASE_PUBLIC_H
#define _EXPRESS_DATABASE_PUBLIC_H

#include "utility/macro.h"
#include <cstdint>
#include <signal.h>

_START_EXPRESS_NAMESPACE_

namespace database
{
    enum class CommCode : uint32_t
    {
        kMakeTable,
        kCreate,
        kRetrieve,
        kUpdate,
        kDelete,
    };

    enum class ExitCode : int
    {
        kSuccess,
        kInvalidArgument,
        kLogInitFailure,
        kLoggedInitFailure,
        kRuntimeFailure,
    };

    inline constexpr int kTerminateSignal = SIGUSR1;
}
  
_END_EXPRESS_NAMESPACE_

#endif