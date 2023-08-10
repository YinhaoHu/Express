#ifndef _EXPRESS_LOG_TEST_SPEC_H
#define _EXPRESS_LOG_TEST_SPEC_H

#include "utility/ipc/message_queue.h"
#include "utility/misc.h"
// This header file can be included in testing. 
// DON'T include it in developing.
#include <bits/stdc++.h>

// For conveience
using namespace std;  
using namespace express::utility::ipc;
using namespace express::utility::misc;

const string mqname("log-test-mq");
constexpr inline size_t maxmsg = 8;
constexpr inline size_t msgsize = 64;

#endif