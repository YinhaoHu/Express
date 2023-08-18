#ifndef _EXPRESS_UTILITY_GENERIC_ARG_PARSE_H
#define _EXPRESS_UTILITY_GENERIC_ARG_PARSE_H

#include "utility/macro.h"
#include <string>

_START_EXPRESS_NAMESPACE_

namespace utility
{
    namespace generic
    {
        class ArgParse
        {
        public:
            ArgParse(int argc, char *argv[]);
            ArgParse(const ArgParse &) = delete;
            ~ArgParse();

            void AddPositional(std::string arg, std::string help);
            void AddOption(std::string short_option, std::string long_option, std::string help);
            std::string GetValue(std::string option);

        private:
            int argc;
            char* argv[];
        
        };
    }
}

_END_EXPRESS_NAMESPACE_

#endif