#include "tool.h"

_START_EXPRESS_NAMESPACE_

namespace test
{
    void test(const char *module, const char *part, bool is_correct)
    {
        cout << format("\033[0;36m[Test]\033[0m{:<16}{:<16}: {}\033[0m\n", module, part,
                       is_correct ? "\033[0;33mPASS" : "\033[0;31mFAIL");
    }
}

_END_EXPRESS_NAMESPACE_