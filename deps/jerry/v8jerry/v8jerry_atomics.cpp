#include "v8jerry_atomics.hpp"
#include "v8jerry_utils.hpp"

namespace JerryAtomics {

/* SharedArrayBuffer */
static jerry_value_t shared_array_buffer_constructor(const jerry_call_info_t *call_info_p,
                                                     const jerry_value_t args_p[],
                                                     const jerry_length_t args_cnt)
{
    if ((args_cnt == 0) || (jerry_value_is_number(args_p[0]) == false)) {
        return jerry_create_undefined();
    }

    jerry_length_t size = (jerry_length_t) jerry_get_number_value(args_p[0]);
    jerry_value_t array = jerry_create_arraybuffer(size);

    // TODO: Create mutex for memory sharing

    return array;
}

/* Atomics */
static jerry_value_t atomics_handler(const jerry_call_info_t *call_info_p,
                                     const jerry_value_t args_p[],
                                     const jerry_length_t args_cnt)
{
    return jerry_create_undefined();
}

/* Atomics.add */
static jerry_value_t atomics_add_handler(const jerry_call_info_t *call_info_p,
                                         const jerry_value_t args_p[],
                                         const jerry_length_t args_cnt)
{
    if (args_cnt < 3) {
        return jerry_create_undefined();
    }

    if ((false == jerry_value_is_typedarray(args_p[0])) ||
        (false == jerry_value_is_number(args_p[1])) ||
        (false == jerry_value_is_number(args_p[2]))) {
        // TODO: add errors handling
        return jerry_create_undefined();
    }

    uint32_t index = (uint32_t)jerry_get_number_value(args_p[1]);
    // TODO: add lock
    jerry_value_t initial_value = jerry_get_property_by_index(args_p[0], index);
    if (jerry_value_is_error(initial_value)) {
        return initial_value;
    }

    double value_num =
        jerry_get_number_value(initial_value) + jerry_get_number_value(args_p[2]);
    jerry_value_t ret = jerry_set_property_by_index(
        args_p[0], index, jerry_create_number(value_num));
    if (jerry_value_is_error(ret)) {
        return ret;
    }
    return initial_value;
}

/* Atomics.load */
static jerry_value_t atomics_load_handler(const jerry_call_info_t *call_info_p,
                                          const jerry_value_t args_p[],
                                          const jerry_length_t args_cnt)
{
    if (args_cnt < 2) {
        return jerry_create_undefined();
    }

    if ((false == jerry_value_is_typedarray(args_p[0])) ||
        (false == jerry_value_is_number(args_p[1]))) {
        // TODO: add errors handling
        return jerry_create_undefined();
    }

    uint32_t index = (uint32_t)jerry_get_number_value(args_p[1]);
    // TODO: add lock
    return jerry_get_property_by_index(args_p[0], index);
}

void InitializeSharedArrayBuffer() {
    JerryxHandlerRegisterGlobal((const jerry_char_t*) "SharedArrayBuffer",
                                    shared_array_buffer_constructor);
    // TODO: Create map of mutexes for memory sharing
}

void Initialize() {
    InitializeSharedArrayBuffer();
    JerryxHandlerRegisterGlobal((const jerry_char_t*)"Atomics",
                                    atomics_handler);
    jerry_value_t global_obj_val = jerry_get_global_object();
    jerry_value_t name_val = jerry_create_string((const jerry_char_t*)"Atomics");
    jerry_value_t atomics_val = jerry_get_property(global_obj_val, name_val);
    jerry_release_value(name_val);
    jerry_release_value(global_obj_val);

    jerry_value_t prototype_val = jerry_get_prototype(atomics_val);
    jerry_release_value(atomics_val);

    // Atomics.add
    JerryxHandlerRegister(
        (const jerry_char_t*)"add", prototype_val, atomics_add_handler);
    // Atomics.load
    JerryxHandlerRegister(
        (const jerry_char_t*)"load", prototype_val, atomics_load_handler);

    jerry_release_value(prototype_val);
    return;
}

}  // namespace JerryAtomics
