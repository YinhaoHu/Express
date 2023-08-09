#ifndef _EXPRESS_UTILITY_MISC_H
#define _EXPRESS_UTILITY_MISC_H

#include "macro.h"
#include <cstdio>
#include <string>
#include <signal.h>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace misc
    {
        template <typename... Ts>
        std::string FormatString(const char *format, size_t maxlen, Ts &&...args)
        {
            char *buf = new char[maxlen]{0};
            snprintf(buf, maxlen, format, std::forward<Ts>(args)...);
            std::string str(buf);
            delete[] buf;

            return str;
        }

        static void print(std::string &&format_str)
        {
            printf("%s", format_str.c_str());
        }

        std::string ErrorString(const char *where)
        {
            return FormatString("%s: %s", 256, where, strerror(errno));
        }
    }
}

_END_EXPRESS_NAMESPACE_

#endif