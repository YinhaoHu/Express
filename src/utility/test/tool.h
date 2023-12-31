#ifndef _EXPRESS_UTILITY_TEST_TOOL_H
#define _EXPRESS_UTILITY_TEST_TOOL_H

#include <bits/stdc++.h>
#include "utility/macro.h"
#include "utility/misc.h"

using namespace std;

_START_EXPRESS_NAMESPACE_

namespace utility::test
{
    template <typename Predicate>
    static void test(const char *module, const char *part, Predicate predic)
    { 
        cout << format("\033[0;36m[Test]\033[0m{:<16}{:<16}: {}\033[0m\n", module, part, 
                        predic() ? "\033[0;33mPASS" : "\033[0;31mFAIL");
    }
 
    void test(const char *module, const char *part, bool is_correct);
}
_END_EXPRESS_NAMESPACE_

#endif