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

#if DEBUG_PRINT

static jerry_value_t
jerryx_handler_print (const jerry_value_t func_obj_val,
                      const jerry_value_t this_p,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt) {
  (void) func_obj_val; /* unused */
  (void) this_p; /* unused */

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
jerryx_handler_string_normalize (const jerry_value_t func_obj_val,
                                 const jerry_value_t this_p,
                                 const jerry_value_t args_p[],
                                 const jerry_length_t args_cnt) {
  (void) func_obj_val; /* unused */
  (void) args_p; /* unused */
  (void) args_cnt; /* unused */

  return jerry_acquire_value(this_p);
}

static char* extract_number(jerry_value_t call_site, jerry_value_t name_string, char* start, char* end) {
    char* num_start = end;
    while (num_start > start && num_start[-1] >= '0' && num_start[-1] <= '9') {
        num_start--;
    }

    char* new_end = num_start;
    uint32_t value = 0;

    while (num_start < end) {
        value = value * 10 + (uint32_t)(*num_start++ - '0');
    }

    jerry_value_t number = jerry_create_number(value);
    jerry_release_value(jerry_set_property(call_site, name_string, number));
    jerry_release_value(number);

    if (new_end > start && new_end[-1] == ':') {
        new_end--;
    }

    return new_end;
}

static void create_structured_trace(jerry_value_t raw_trace) {
    char buf[128];
    uint32_t length = jerry_get_array_length(raw_trace);
    jerry_value_t line_number_string = jerry_create_string((const jerry_char_t*)"lineNumber_");
    jerry_value_t file_name_string = jerry_create_string((const jerry_char_t*)"fileName_");
    JerryValue *prototype = JerryIsolate::GetCurrent()->CallSitePrototype();

    for (uint32_t i = 0; i < length; i++) {
        jerry_value_t call_site = jerry_create_object();
        jerry_release_value(jerry_set_prototype(call_site, prototype->value()));

        jerry_value_t str_value = jerry_get_property_by_index(raw_trace, i);
        jerry_size_t size = jerry_get_string_size(str_value);
        char* start;

        if (size <= sizeof(buf)) {
            start = buf;
        } else {
            start = (char*)malloc(size);
        }

        jerry_string_to_char_buffer(str_value, (jerry_char_t*)start, size);
        jerry_release_value(str_value);
        char* end = start + size;

        end = extract_number(call_site, line_number_string, start, end);

        jerry_value_t file_name = jerry_create_string_sz((jerry_char_t*)start, (jerry_size_t)(end - start));
        jerry_release_value(jerry_set_property(call_site, file_name_string, file_name));
        jerry_release_value(file_name);

        if (size > sizeof(buf)) {
            free(start);
        }

        jerry_release_value(jerry_set_property_by_index(raw_trace, i, call_site));
        jerry_release_value(call_site);
    }

    jerry_release_value(line_number_string);
    jerry_release_value(file_name_string);
}

static jerry_value_t JerryHandlerStackTraceGetter(const jerry_value_t func,
                                                  const jerry_value_t this_val,
                                                  const jerry_value_t args_p[],
                                                  const jerry_length_t args_count)
{
    jerry_value_t raw_string = jerry_create_string((const jerry_char_t*)"raw");
    JerryValue raw_trace(jerry_get_property(func, raw_string));

    if (jerry_value_is_undefined(raw_trace.value()))
    {
      jerry_release_value(raw_string);

      jerry_value_t value_string = jerry_create_string((const jerry_char_t*)"value");
      jerry_value_t value_result = jerry_get_property(func, value_string);
      jerry_release_value(value_string);
      return value_result;
    }

    jerry_delete_property(func, raw_string);
    jerry_release_value(raw_string);

    JerryIsolate* isolate = JerryIsolate::GetCurrent();
    v8::HandleScope handle_scope(JerryIsolate::toV8(isolate));

    v8::PrepareStackTraceCallback callback = isolate->PrepareStackTraceCallback();
    assert(callback);

    JerryValue jerry_context(jerry_acquire_value(isolate->CurrentContext()->value()));
    v8::Local<v8::Context> context = jerry_context.AsLocal<v8::Context>();

    JerryValue jerry_error(jerry_acquire_value(this_val));
    v8::Local<v8::Value> error = jerry_error.AsLocal<v8::Value>();

    create_structured_trace(raw_trace.value());
    v8::Local<v8::Array> sites = raw_trace.AsLocal<v8::Array>();

    v8::MaybeLocal<v8::Value> result = callback(context, error, sites);

    jerry_value_t final_result;

    if (isolate->HasError()) {
        JerryValue* jerror = isolate->GetRawError();
        final_result = jerry_create_error_from_value(jerror->value(), false);
        isolate->ClearError(NULL);
    } else if (!result.IsEmpty()) {
        JerryValue* result_value = *reinterpret_cast<JerryValue**>(&result);
        final_result = jerry_acquire_value(result_value->value());
    } else {
        final_result = jerry_create_undefined();
    }

    jerry_value_t value_string = jerry_create_string((const jerry_char_t*)"value");
    jerry_value_t value_result = jerry_set_property(func, value_string, final_result);
    jerry_release_value(value_string);
    return final_result;
}

static jerry_value_t JerryHandlerStackTraceSetter(const jerry_value_t func,
                                                  const jerry_value_t this_val,
                                                  const jerry_value_t args_p[],
                                                  const jerry_length_t args_count)
{
    jerry_value_t getter_string = jerry_create_string((const jerry_char_t*)"getter");
    jerry_value_t getter_value = jerry_get_property(func, getter_string);
    jerry_release_value(getter_string);

    jerry_value_t raw_string = jerry_create_string((const jerry_char_t*)"raw");
    jerry_delete_property(getter_value, raw_string);
    jerry_release_value(raw_string);

    jerry_value_t value_string = jerry_create_string((const jerry_char_t*)"value");
    jerry_value_t set_result = jerry_set_property(getter_value, value_string, args_p[0]);
    jerry_release_value(set_result);
    jerry_release_value(value_string);

    jerry_release_value(getter_value);
    return jerry_create_undefined();
}

static jerry_value_t JerryHandlerStackTrace(const jerry_value_t func,
                                            const jerry_value_t this_val,
                                            const jerry_value_t args_p[],
                                            const jerry_length_t args_count)
{
    if (!jerry_is_feature_enabled(JERRY_FEATURE_LINE_INFO) || args_count == 0 || !jerry_value_is_object(args_p[0])) {
        return jerry_create_undefined();
    }

    jerry_value_t global_object = jerry_get_global_object();
    jerry_value_t error_string = jerry_create_string((const jerry_char_t*)"Error");
    jerry_value_t error_result = jerry_get_property(global_object, error_string);
    jerry_release_value(error_string);
    jerry_release_value(global_object);

    if (jerry_value_is_error(error_result)) {
        return error_result;
    }

    jerry_value_t limit_string = jerry_create_string((const jerry_char_t*)"stackTraceLimit");
    jerry_value_t limit_result = jerry_get_property(error_result, limit_string);
    jerry_release_value(limit_string);
    jerry_release_value(error_result);

    if (jerry_value_is_error(limit_result)) {
        return limit_result;
    }

    uint32_t max_depth = 0;

    if (jerry_value_is_number(limit_result)) {
        double depth = jerry_get_number_value(limit_result);
        if (depth < 1) {
            max_depth = UINT32_MAX;
        } else if (depth < UINT32_MAX) {
            max_depth = (uint32_t)depth;
        }
    }
    jerry_release_value(limit_result);

    // https://v8.dev/docs/stack-trace-api
    jerry_value_t stack_string = jerry_create_string((const jerry_char_t*)"stack");

    jerry_property_descriptor_t prop_desc;
    jerry_init_property_descriptor_fields(&prop_desc);

    prop_desc.is_get_defined = true;
    prop_desc.getter = jerry_create_external_function(JerryHandlerStackTraceGetter);
    prop_desc.is_set_defined = true;
    prop_desc.setter = jerry_create_external_function(JerryHandlerStackTraceSetter);

    jerry_value_t stack_trace;
    if (max_depth != UINT32_MAX) {
        if (args_count >= 2) {
            stack_trace = jerry_get_backtrace_from(max_depth, args_p[1]);
        } else {
            stack_trace = jerry_get_backtrace(max_depth);
        }
    } else {
        stack_trace = jerry_create_array(0);
    }

    jerry_value_t raw_string = jerry_create_string((const jerry_char_t*)"raw");
    jerry_value_t set_result = jerry_set_property(prop_desc.getter, raw_string, stack_trace);
    jerry_release_value(set_result);
    jerry_release_value(raw_string);
    jerry_release_value(stack_trace);

    jerry_value_t getter_string = jerry_create_string((const jerry_char_t*)"getter");
    set_result = jerry_set_property(prop_desc.setter, getter_string, prop_desc.getter);
    jerry_release_value(set_result);
    jerry_release_value(getter_string);

    prop_desc.is_enumerable_defined = true;
    prop_desc.is_enumerable = false;
    prop_desc.is_configurable_defined = true;
    prop_desc.is_configurable = true;

    jerry_define_own_property(args_p[0], stack_string, &prop_desc);
    jerry_free_property_descriptor_fields(&prop_desc);
    jerry_release_value(stack_string);

    return jerry_create_undefined();
}

static jerry_value_t JerryHandlerGC(const jerry_value_t func,
                                    const jerry_value_t thisarg,
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
    JerryValue capture_stack_trace_string(jerry_create_string((const jerry_char_t*)"captureStackTrace"));
    JerryValue stack_trace_function(jerry_create_external_function(JerryHandlerStackTrace));

    JerryValue* error_obj = global.GetProperty(&error_string);
    if (error_obj == NULL) {
        printf("Error object is not defined...\n");
        abort();
    }

    error_obj->SetProperty(&capture_stack_trace_string, &stack_trace_function);
    delete error_obj;
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
    jerry_property_descriptor_t desc;
    jerry_init_property_descriptor_fields(&desc);
    desc.is_value_defined = true;
    desc.value = function_val;

    jerry_value_t result_val = jerry_define_own_property (object_value, function_name_val, &desc);

    jerry_free_property_descriptor_fields (&desc);
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
