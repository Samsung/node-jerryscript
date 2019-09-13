#ifndef V8JERRY_CALLBACK_HPP
#define V8JERRY_CALLBACK_HPP

#include <jerryscript.h>

class JerryV8FunctionHandlerData;

jerry_value_t JerryV8GetterSetterHandler(
    const jerry_value_t function_obj,
    const jerry_value_t this_val,
    const jerry_value_t args_p[],
    const jerry_length_t args_cnt);


JerryV8FunctionHandlerData* JerryGetFunctionHandlerData(jerry_value_t target);

jerry_value_t JerryV8FunctionHandler(
    const jerry_value_t function_obj,
    const jerry_value_t this_val,
    const jerry_value_t args_p[],
    const jerry_length_t args_cnt);


#endif /* V8JERRY_CALLBACK_HPP */
