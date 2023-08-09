#include "signal.h"
#include <cstring>

_START_EXPRESS_NAMESPACE_

namespace utility::ipc
{
    sighandler_t Signal(int signo, sighandler_t sighandler) noexcept
    {
        struct sigaction act, oact;

        memset(&act, 0, sizeof(act));
        act.sa_flags = 0;
        act.sa_handler = sighandler;
        sigemptyset(&act.sa_mask);

        if (signo == SIGALRM)
            act.sa_flags |= SA_INTERRUPT;
        else
            act.sa_flags |= SA_RESTART;

        if (sigaction(signo, &act, &oact) < 0)
            return SIG_ERR;
        return oact.sa_handler;
    }
}

_END_EXPRESS_NAMESPACE_