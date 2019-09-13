#ifndef V8JERRY_UTILS_HPP
#define V8JERRY_UTILS_HPP

#include <cstring>
#include <cstdlib> // TODO: remove

#include <jerryscript.h>

/* Tooling helper methods*/
jerry_value_t BuildHelperMethod(const char* fn_args, const char* fn_body);

#endif
