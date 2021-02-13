#ifndef V8JERRY_CALLBACK_HPP
#define V8JERRY_CALLBACK_HPP

#include <jerryscript.h>

class JerryV8FunctionHandlerData;

jerry_value_t JerryV8GetterSetterHandler(
    const jerry_call_info_t *call_info_p,
    const jerry_value_t args_p[],
    const jerry_length_t args_cnt);


JerryV8FunctionHandlerData* JerryGetFunctionHandlerData(jerry_value_t target);

jerry_value_t JerryV8FunctionHandler(
    const jerry_call_info_t *call_info_p,
    const jerry_value_t args_p[],
    const jerry_length_t args_cnt);

jerry_value_t JerryV8ProxyHandler(
    const jerry_call_info_t *call_info_p,
    const jerry_value_t args_p[],
    const jerry_length_t args_cnt);


#endif /* V8JERRY_CALLBACK_HPP */
