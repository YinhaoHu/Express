#ifndef _EXPRESS_UTILITY_MACRO_H
#define _EXPRESS_UTILITY_MACRO_H

#if __cplusplus < 202002L
#error "Express compilation requires at least C++ 20."
#endif
 
#define _START_EXPRESS_NAMESPACE_ namespace express{
#define _END_EXPRESS_NAMESPACE_ } //namespace express


/**
 * Debug macro is : _EXPRESS_DEBUG
 * It is defined in compile commond line which can be seen in Makefile.
*/

#ifdef _EXPRESS_DEBUG
#define _EXPRESS_DEBUG_INSTRUCTION(expression) expression
#include <syslog.h>
#else 
#define _EXPRESS_DEBUG_INSTRUCTION(expression) 
#endif
#endif