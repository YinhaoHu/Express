#ifndef _EXPRESS_UTILITY_MISC_H
#define _EXPRESS_UTILITY_MISC_H

#include "macro.h"
#include <system_error>
#include <format>
#include <cstdio>
#include <cstring>
#include <string>
#include <signal.h>

#define SYSTEM_ERROR_INFO(where_cstring) __FILE__,__LINE__,where_cstring  

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace misc
    {
        template <typename... Ts>
        static std::string FormatString(const char *format, size_t maxlen, Ts &&...args)
        {
            char *buf = new char[maxlen]{0};
            snprintf(buf, maxlen, format, std::forward<Ts>(args)...);
            std::string str(buf);
            delete[] buf;

            return str;
        }

        inline void print(std::string &&format_str)
        {
            printf("%s", format_str.c_str());
        }

        inline std::string ErrorString(const char *where)
        {
            return FormatString("%s: %s", 256, where, strerror(errno));
        }


        inline void ThrowSystemError(const char* file_name, int line, const char* func)
        {
            // This is NOT error but a issue of support for format on VSCode.
            throw std::system_error(errno, std::generic_category(),
            std::format("{} {} {}", file_name, func, line ) 
            );
        }
    }
}

_END_EXPRESS_NAMESPACE_

#endif