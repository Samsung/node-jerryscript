#ifndef V8JERRY_UTILS_HPP
#define V8JERRY_UTILS_HPP

#include <jerryscript.h>

/* Tooling helper methods*/
class JerryPolyfill {
public:
    JerryPolyfill(const char* name, const char* fn_args, const char* fn_body);
    ~JerryPolyfill(void);

    jerry_value_t Call(const jerry_value_t this_arg, const jerry_value_t *args = NULL, int argc = 0) const;

private:
    static jerry_value_t BuildMethod(const char* name, const char* fn_args, const char* fn_body);

    jerry_value_t m_method;
};

void JerryForceCleanup(void);

#endif
