#ifndef _EXPRESS_UTILITI_IPC_SIGNAL_H
#define _EXPRESS_UTILITI_IPC_SIGNAL_H

#include "utility/macro.h"
#include <signal.h>

_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    sighandler_t Signal(int signo, sighandler_t sighandler) noexcept;
}

_END_EXPRESS_NAMESPACE_

#endif