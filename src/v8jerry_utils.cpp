#include "v8jerry_utils.hpp"

#include <cstdlib>
#include <cstring>

JerryPolyfill::JerryPolyfill(const char* name, const char* fn_args, const char* fn_body)
    : m_method(JerryPolyfill::BuildMethod(name, fn_args, fn_body))
{
}

JerryPolyfill::~JerryPolyfill() {
    jerry_release_value(m_method);
}

jerry_value_t JerryPolyfill::Call(const jerry_value_t this_arg, const jerry_value_t *args, int argc) const {
    jerry_value_t result = jerry_call_function(m_method, this_arg, args, argc);
    return result;
}

jerry_value_t JerryPolyfill::BuildMethod(const char* name, const char* fn_args, const char* fn_body) {
    jerry_value_t method = jerry_parse_function(reinterpret_cast<const jerry_char_t*>(name), strlen(name),
                                 reinterpret_cast<const jerry_char_t*>(fn_args), strlen(fn_args),
                                 reinterpret_cast<const jerry_char_t*>(fn_body), strlen(fn_body),
                                 JERRY_PARSE_NO_OPTS);
    if (jerry_value_is_error(method)) {
        fprintf(stderr, "Failed to build helper method initialize at: %s:%d\nfunction (%s) {\n%s\n}", __FILE__, __LINE__, fn_args, fn_body);
        abort();
    }

    return method;
}
