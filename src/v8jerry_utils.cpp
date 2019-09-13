#include "v8jerry_utils.hpp"

#include <cstdlib>

jerry_value_t BuildHelperMethod(const char* fn_args, const char* fn_body) {
    jerry_value_t method = jerry_parse_function(NULL, 0,
                                 reinterpret_cast<const jerry_char_t*>(fn_args), strlen(fn_args),
                                 reinterpret_cast<const jerry_char_t*>(fn_body), strlen(fn_body),
                                 JERRY_PARSE_NO_OPTS);
    if (jerry_value_is_error(method)) {
        fprintf(stderr, "Failed to build helper method initialize at: %s:%d\nfunction (%s) {\n%s\n}", __FILE__, __LINE__, fn_args, fn_body);
        abort();
    }
    return method;
}

