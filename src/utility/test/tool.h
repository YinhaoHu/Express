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
        using namespace misc;
        print(FormatString("[Test]%s-%s: %s\033[0m\n", 512,
                           module, part, predic() ? "\033[0;33mPASS" : "\033[0;31mFAIL"));
    }
}
_END_EXPRESS_NAMESPACE_

#endif