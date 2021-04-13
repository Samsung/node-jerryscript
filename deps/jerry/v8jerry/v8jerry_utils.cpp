#include "v8jerry_utils.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "v8jerry_atomics.hpp"
#include "v8jerry_flags.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_value.hpp"

#define DEBUG_PRINT 1

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
    jerry_parse_options_t parse_options;
    parse_options.options = JERRY_PARSE_HAS_RESOURCE;
    parse_options.resource_name_p = reinterpret_cast<const jerry_char_t*>(name);
    parse_options.resource_name_length = strlen(name);

    jerry_value_t method = jerry_parse_function(reinterpret_cast<const jerry_char_t*>(fn_args), strlen(fn_args),
                                 reinterpret_cast<const jerry_char_t*>(fn_body), strlen(fn_body),
                                 &parse_options);
    if (jerry_value_is_error(method)) {
        fprintf(stderr, "Failed to build helper method initialize at: %s:%d\nfunction (%s) {\n%s\n}", __FILE__, __LINE__, fn_args, fn_body);
        abort();
    }

    return method;
}

#if DEBUG_PRINT

static jerry_value_t
jerryx_handler_print (const jerry_call_info_t *call_info_p,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt) {
  (void) call_info_p; /* unused */

  const char * const null_str = "\\u0000";

  jerry_value_t ret_val = jerry_create_undefined ();

  for (jerry_length_t arg_index = 0; arg_index < args_cnt; arg_index++)
  {
    jerry_value_t str_val;

    if (jerry_value_is_symbol (args_p[arg_index]))
    {
      str_val = jerry_get_symbol_descriptive_string (args_p[arg_index]);
    }
    else
    {
      str_val = jerry_value_to_string (args_p[arg_index]);
    }

    if (jerry_value_is_error (str_val))
    {
      /* There is no need to free the undefined value. */
      ret_val = str_val;
      break;
    }

    jerry_length_t length = jerry_get_utf8_string_length (str_val);
    jerry_length_t substr_pos = 0;
    jerry_char_t substr_buf[256];

    do
    {
      jerry_size_t substr_size = jerry_substring_to_utf8_char_buffer (str_val,
                                                                      substr_pos,
                                                                      length,
                                                                      substr_buf,
                                                                      256 - 1);

      jerry_char_t *buf_end_p = substr_buf + substr_size;

      /* Update start position by the number of utf-8 characters. */
      for (jerry_char_t *buf_p = substr_buf; buf_p < buf_end_p; buf_p++)
      {
        /* Skip intermediate utf-8 octets. */
        if ((*buf_p & 0xc0) != 0x80)
        {
          substr_pos++;
        }
      }

      if (substr_pos == length)
      {
        *buf_end_p++ = (arg_index < args_cnt - 1) ? ' ' : '\n';
      }

      for (jerry_char_t *buf_p = substr_buf; buf_p < buf_end_p; buf_p++)
      {
        char chr = (char) *buf_p;

        if (chr != '\0')
        {
          jerry_port_print_char (chr);
          continue;
        }

        for (jerry_size_t null_index = 0; null_str[null_index] != '\0'; null_index++)
        {
          jerry_port_print_char (null_str[null_index]);
        }
      }
    }
    while (substr_pos < length);

    jerry_release_value (str_val);
  }

  if (args_cnt == 0 || jerry_value_is_error (ret_val))
  {
    jerry_port_print_char ('\n');
  }

  return ret_val;
}

#endif

static jerry_value_t
jerryx_handler_string_normalize (const jerry_call_info_t *call_info_p,
                                 const jerry_value_t args_p[],
                                 const jerry_length_t args_cnt) {
    (void) args_p; /* unused */
    (void) args_cnt; /* unused */

    return jerry_acquire_value(call_info_p->this_value);
}

struct StackFrame {
    jerry_value_t resource;
    jerry_size_t line;
    jerry_size_t column;
};

struct StackData {
    std::vector<StackFrame> frames;
};

void StackDataFree(void* native_p, jerry_object_native_info_t* info_p) {
    (void) info_p;

    StackData* data = reinterpret_cast<StackData*>(native_p);

    std::vector<StackFrame>::iterator end = data->frames.end();
    for (std::vector<StackFrame>::iterator it = data->frames.begin(); it != end; ++it) {
        jerry_release_value(it->resource);
    }

    delete data;
}

static jerry_object_native_info_t StackDataTypeInfo = {
    .free_cb = StackDataFree,
    .number_of_references = 0,
    .offset_of_references = 0,
};

static jerry_value_t JerryHandlerStackTraceGetter(const jerry_call_info_t *call_info_p,
                                                  const jerry_value_t args_p[],
                                                  const jerry_length_t args_count)
{
    jerry_value_t function = call_info_p->function;
    jerry_value_t value_string = jerry_create_string((const jerry_char_t*)"value");
    jerry_value_t value_result = jerry_get_property(call_info_p->function, value_string);
    jerry_release_value(value_string);

    void *native_p;
    if (!jerry_get_object_native_pointer(function, &native_p, &StackDataTypeInfo)) {
        return value_result;
    }

    jerry_delete_object_native_pointer(function, &StackDataTypeInfo);

    StackData* data = reinterpret_cast<StackData*>(native_p);

    jerry_value_t resource_string = jerry_create_string((const jerry_char_t*)"resource__");
    jerry_value_t line_string = jerry_create_string((const jerry_char_t*)"line__");
    jerry_value_t function_string = jerry_create_string((const jerry_char_t*)"function__");
    JerryValue *prototype = JerryIsolate::GetCurrent()->CallSitePrototype();
    uint32_t index = 0;

    std::vector<StackFrame>::iterator end = data->frames.end();
    for (std::vector<StackFrame>::iterator it = data->frames.begin(); it != end; ++it) {
        jerry_value_t call_site = jerry_create_object();
        jerry_release_value(jerry_set_prototype(call_site, prototype->value()));

        jerry_value_t called_function = jerry_get_property_by_index(value_result, index);

        jerry_release_value(jerry_set_property(call_site, function_string, called_function));
        jerry_release_value(called_function);

        jerry_release_value(jerry_set_property(call_site, resource_string, it->resource));
        jerry_value_t line = jerry_create_number(it->line);
        jerry_release_value(jerry_set_property(call_site, line_string, line));
        jerry_release_value(line);

        jerry_release_value(jerry_set_property_by_index(value_result, index, call_site));
        jerry_release_value(call_site);
        index++;
    }

    jerry_release_value(function_string);
    jerry_release_value(line_string);
    jerry_release_value(resource_string);

    StackDataFree(native_p, NULL);

    JerryIsolate* isolate = JerryIsolate::GetCurrent();
    v8::PrepareStackTraceCallback callback = isolate->PrepareStackTraceCallback();
    assert(callback);

    JerryValue jerry_context(jerry_acquire_value(isolate->CurrentContext()->value()));
    v8::Local<v8::Context> context = jerry_context.AsLocal<v8::Context>();

    JerryValue jerry_error(jerry_acquire_value(call_info_p->this_value));
    v8::Local<v8::Value> error = jerry_error.AsLocal<v8::Value>();

    /* Pass ownership. */
    JerryValue jerry_sites(value_result);
    v8::Local<v8::Array> sites = jerry_sites.AsLocal<v8::Array>();

    v8::MaybeLocal<v8::Value> result = callback(context, error, sites);

    jerry_value_t final_result;

    if (isolate->HasError()) {
        final_result = jerry_create_error_from_value(isolate->TakeError(), true);
    } else if (!result.IsEmpty()) {
        JerryValue* result_value = *reinterpret_cast<JerryValue**>(&result);
        final_result = jerry_acquire_value(result_value->value());
    } else {
        final_result = jerry_create_undefined();
    }

    value_string = jerry_create_string((const jerry_char_t*)"value");
    jerry_release_value(jerry_set_property(function, value_string, final_result));
    jerry_release_value(value_string);
    return final_result;
}

static jerry_value_t JerryHandlerStackTraceSetter(const jerry_call_info_t *call_info_p,
                                                  const jerry_value_t args_p[],
                                                  const jerry_length_t args_count)
{
    jerry_value_t getter_string = jerry_create_string((const jerry_char_t*)"getter");
    jerry_value_t getter_value = jerry_get_property(call_info_p->function, getter_string);
    jerry_release_value(getter_string);

    void *native_p;
    if (jerry_get_object_native_pointer(getter_value, &native_p, &StackDataTypeInfo)) {
        jerry_delete_object_native_pointer(getter_value, &StackDataTypeInfo);
        StackDataFree(native_p, NULL);
    }

    jerry_value_t value_string = jerry_create_string((const jerry_char_t*)"value");
    jerry_value_t set_result = jerry_set_property(getter_value, value_string, args_p[0]);
    jerry_release_value(set_result);
    jerry_release_value(value_string);

    jerry_release_value(getter_value);
    return jerry_create_undefined();
}

typedef struct {
    StackData *data;
    const jerry_value_t *ignored_function;
    jerry_value_t undefined;
    jerry_value_t function_list;
    uint32_t max_frames;
    uint32_t frame_count;
} BacktraceCallbackData;

static bool backtrace_callback(jerry_backtrace_frame_t *frame_p, void *user_p) {
    BacktraceCallbackData *callback_data = reinterpret_cast<BacktraceCallbackData*>(user_p);

    const jerry_value_t *function = jerry_backtrace_get_function(frame_p);

    if (callback_data->ignored_function != NULL) {
        if (function != NULL && *function == *callback_data->ignored_function) {
            callback_data->ignored_function = NULL;
        }
        return true;
    }

    StackFrame frame;

    const jerry_backtrace_location_t *location = jerry_backtrace_get_location(frame_p);
    if (location != NULL) {
        frame.resource = jerry_acquire_value(location->resource_name);
        frame.line = location->line;
        frame.column = location->column;
    } else {
        frame.resource = jerry_create_undefined();
        frame.line = 1;
        frame.column = 1;
    }

    jerry_value_t prop = (function != NULL) ? *function : callback_data->undefined;
    jerry_set_property_by_index (callback_data->function_list, callback_data->frame_count, prop);

    callback_data->data->frames.push_back(frame);
    return ++callback_data->frame_count < callback_data->max_frames;
}

void CreateStackTrace(const jerry_value_t object, const jerry_value_t *ignored_function) {
    BacktraceCallbackData callback_data;
    callback_data.ignored_function = ignored_function;
    callback_data.max_frames = UINT32_MAX;
    callback_data.frame_count = 0;

    jerry_value_t global_object = jerry_get_global_object();
    jerry_value_t error_string = jerry_create_string((const jerry_char_t*)"Error");
    jerry_value_t error_result = jerry_get_property(global_object, error_string);
    jerry_release_value(error_string);
    jerry_release_value(global_object);

    if (!jerry_value_is_error(error_result)) {
        jerry_value_t limit_string = jerry_create_string((const jerry_char_t*)"stackTraceLimit");
        jerry_value_t limit_result = jerry_get_property(error_result, limit_string);
        jerry_release_value(limit_string);

        if (!jerry_value_is_error(limit_result)) {
            if (jerry_value_is_number(limit_result)) {
                double depth = jerry_get_number_value(limit_result);

                if (depth >= 0 && depth < UINT32_MAX) {
                    callback_data.max_frames = (uint32_t)depth;
                }
            }
        }
        jerry_release_value(limit_result);
    }
    jerry_release_value(error_result);

    // https://v8.dev/docs/stack-trace-api
    jerry_property_descriptor_t prop_desc = jerry_property_descriptor_create();
    prop_desc.flags = (JERRY_PROP_IS_GET_DEFINED
                       | JERRY_PROP_IS_SET_DEFINED
                       | JERRY_PROP_IS_CONFIGURABLE_DEFINED
                       | JERRY_PROP_IS_CONFIGURABLE
                       | JERRY_PROP_IS_ENUMERABLE_DEFINED);

    prop_desc.getter = jerry_create_external_function(JerryHandlerStackTraceGetter);
    prop_desc.setter = jerry_create_external_function(JerryHandlerStackTraceSetter);

    callback_data.data = new StackData();
    callback_data.function_list = jerry_create_array(0);
    callback_data.undefined = jerry_create_undefined();

    if (callback_data.max_frames > 0) {
        jerry_backtrace_capture(backtrace_callback, reinterpret_cast<void*>(&callback_data));
    }

    jerry_release_value(callback_data.undefined);

    jerry_set_object_native_pointer(prop_desc.getter, callback_data.data, &StackDataTypeInfo);

    jerry_value_t value_string = jerry_create_string((const jerry_char_t*)"value");
    jerry_set_property(prop_desc.getter, value_string, callback_data.function_list);
    jerry_release_value(value_string);
    jerry_release_value(callback_data.function_list);

    jerry_value_t getter_string = jerry_create_string((const jerry_char_t*)"getter");
    jerry_value_t set_result = jerry_set_property(prop_desc.setter, getter_string, prop_desc.getter);
    jerry_release_value(set_result);
    jerry_release_value(getter_string);

    jerry_value_t stack_string = jerry_create_string((const jerry_char_t*)"stack");
    jerry_define_own_property(object, stack_string, &prop_desc);
    jerry_property_descriptor_free(&prop_desc);
    jerry_release_value(stack_string);
}

static jerry_value_t JerryHandlerStackTrace(const jerry_call_info_t *call_info_p,
                                            const jerry_value_t args_p[],
                                            const jerry_length_t args_count)
{
    if (args_count == 0 || !jerry_value_is_object(args_p[0])) {
        return jerry_create_undefined();
    }

    CreateStackTrace(args_p[0], (const jerry_value_t*)(args_count >= 2 ? (args_p + 1) : NULL));
    return jerry_create_undefined();
}

static jerry_value_t JerryHandlerGC(const jerry_call_info_t *call_info_p,
                                    const jerry_value_t argv[],
                                    const jerry_value_t argc) {
    jerry_gc (JERRY_GC_PRESSURE_LOW);
    return jerry_create_undefined();
}

void InjectGlobalFunctions(void) {
#if DEBUG_PRINT
    JerryxHandlerRegisterGlobal((const jerry_char_t *)"print", jerryx_handler_print);
#endif
    JerryxHandlerRegisterString((const jerry_char_t *)"normalize", jerryx_handler_string_normalize);

    JerryAtomics::Initialize();

    JerryValue global(jerry_get_global_object());

    if (Flag::Get(Flag::expose_gc)->u.bool_value) {
        JerryValue gc_string(jerry_create_string((const jerry_char_t*)"gc"));
        JerryValue gc_function(jerry_create_external_function(JerryHandlerGC));

        global.SetProperty(&gc_string, &gc_function);
    }

    JerryValue error_string(jerry_create_string((const jerry_char_t*)"Error"));

    JerryValue* error_obj = global.GetProperty(&error_string);
    if (error_obj != NULL) {
        JerryValue capture_stack_trace_string(jerry_create_string((const jerry_char_t*)"captureStackTrace"));
        JerryValue stack_trace_value(jerry_create_external_function(JerryHandlerStackTrace));

        error_obj->SetProperty(&capture_stack_trace_string, &stack_trace_value);

        JerryValue stack_trace_limit_string(jerry_create_string((const jerry_char_t*)"stackTraceLimit"));
        JerryValue stack_trace_limit_value(jerry_create_number(10));

        error_obj->SetProperty(&stack_trace_limit_string, &stack_trace_limit_value);
        delete error_obj;
    }
}

// TODO: remove these layering violations (this is a Jerry internal method, should not be visible here)
extern "C" bool ecma_get_object_is_builtin(void* obj);
#define ECMA_OBJECT_REF_ONE (1u << 6)
#define ECMA_OBJECT_MAX_REF (0x3ffu << 6)
#define ECMA_VALUE_TYPE_MASK 0x7u
#define ECMA_VALUE_SHIFT 3
typedef uint32_t ecma_value_t;

extern "C" void * jmem_decompress_pointer (uintptr_t compressed_pointer);

typedef struct {
  /** type : 4 bit : ecma_object_type_t or ecma_lexical_environment_type_t
                     depending on ECMA_OBJECT_FLAG_BUILT_IN_OR_LEXICAL_ENV
      flags : 2 bit : ECMA_OBJECT_FLAG_BUILT_IN_OR_LEXICAL_ENV,
                      ECMA_OBJECT_FLAG_EXTENSIBLE or ECMA_OBJECT_FLAG_NON_CLOSURE
      refs : 10 bit (max 1023) */
  uint16_t type_flags_refs;
  /* snippet from JerryScript code */
} header_ecma_object_t;

static void *ecma_get_pointer_from_ecma_value (ecma_value_t value) {
#if __UINTPTR_MAX__ <= __UINT32_MAX__
  void *ptr = (void *) (uintptr_t) ((value) & ~ECMA_VALUE_TYPE_MASK);
#else
  void *ptr = (void*)jmem_decompress_pointer(value >> ECMA_VALUE_SHIFT);
#endif
  return ptr;
}

static bool collect_objects(const jerry_value_t object, void *user_data_p) {
    std::vector<jerry_value_t>* objects = reinterpret_cast<std::vector<jerry_value_t>*>(user_data_p);

    objects->push_back(object);

    return true;
}

static bool ecam_have_ref(const jerry_value_t value) {
    header_ecma_object_t* header = reinterpret_cast<header_ecma_object_t*>(ecma_get_pointer_from_ecma_value(value));
    return header->type_flags_refs >= ECMA_OBJECT_REF_ONE;
}

void JerryForceCleanup(void) {
    jerry_gc(JERRY_GC_PRESSURE_HIGH);

    std::vector<jerry_value_t> objects;
    jerry_objects_foreach(collect_objects, &objects);

    for (size_t idx = 0; idx < objects.size(); idx++) {
        bool is_builtin = ecma_get_object_is_builtin(ecma_get_pointer_from_ecma_value(objects[idx]));
        if (!is_builtin && ecam_have_ref(objects[idx])) {
            jerry_release_value(objects[idx]);
        }
    }
}

void JerryxHandlerRegister (const jerry_char_t *name_p, jerry_value_t object_value,
                            jerry_external_handler_t handler_p) {
    jerry_value_t function_name_val = jerry_create_string (name_p);
    jerry_value_t function_val = jerry_create_external_function (handler_p);
    jerry_property_descriptor_t desc = jerry_property_descriptor_create();
    desc.flags = (JERRY_PROP_IS_VALUE_DEFINED
                  | JERRY_PROP_IS_CONFIGURABLE_DEFINED
                  | JERRY_PROP_IS_CONFIGURABLE
                  | JERRY_PROP_IS_ENUMERABLE_DEFINED
                  | JERRY_PROP_IS_WRITABLE_DEFINED
                  | JERRY_PROP_IS_WRITABLE);
    desc.value = function_val;

    jerry_value_t result_val = jerry_define_own_property (object_value, function_name_val, &desc);

    jerry_property_descriptor_free (&desc);
    jerry_release_value (function_name_val);

    if (jerry_value_is_error (result_val))
    {
        jerry_port_log (JERRY_LOG_LEVEL_WARNING, "Warning: failed to register '%s' method.", name_p);
    }

    jerry_release_value (result_val);
}

void JerryxHandlerRegisterGlobal (const jerry_char_t *name_p,
                                  jerry_external_handler_t handler_p) {
    jerry_value_t global_obj_val = jerry_get_global_object();
    JerryxHandlerRegister(name_p, global_obj_val, handler_p);
    jerry_release_value(global_obj_val);
}

void JerryxHandlerRegisterString (const jerry_char_t *name_p,
                                  jerry_external_handler_t handler_p) {
    jerry_value_t global_obj_val = jerry_get_global_object();
    jerry_value_t name_val = jerry_create_string((const jerry_char_t *) "String");
    jerry_value_t string_val = jerry_get_property(global_obj_val, name_val);
    jerry_release_value (name_val);
    jerry_release_value (global_obj_val);

    name_val = jerry_create_string((const jerry_char_t *) "prototype");
    jerry_value_t prototype_val = jerry_get_property(string_val, name_val);
    jerry_release_value (name_val);
    jerry_release_value (string_val);

    JerryxHandlerRegister (name_p, prototype_val, handler_p);
    jerry_release_value (prototype_val);
}
