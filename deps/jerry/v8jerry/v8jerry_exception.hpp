#ifndef V8JERRY_EXCEPTION_HPP
#define V8JERRY_EXCEPTION_HPP

#include <jerryscript.h>
#include <v8.h>

void JerryThrowCallback(const jerry_value_t error_value, void* user_p);
void JerryThrowSyntaxError(jerry_value_t error_value, jerry_value_t source_string, jerry_parse_options_t* parse_options, void* user_p);

int JerryMessageGetLineNumber(jerry_value_t message_value);
int JerryMessageGetColumnStartNumber(jerry_value_t message_value);
int JerryMessageGetColumnEndNumber(jerry_value_t message_value);
jerry_value_t JerryMessageGetScriptResourceName(jerry_value_t message_value);
jerry_value_t JerryMessageGetSourceLine(jerry_value_t message_value);

#endif /* V8JERRY_EXCEPTION_HPP */
