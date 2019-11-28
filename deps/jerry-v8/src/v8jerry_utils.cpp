#include "v8jerry_utils.hpp"

#include <cstdlib>
#include <cstring>
#include <vector>

#include "v8jerry_flags.hpp"
#include "v8jerry_value.hpp"

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

static jerry_value_t JerryHandlerGC(const jerry_value_t func,
                                    const jerry_value_t thisarg,
                                    const jerry_value_t argv[],
                                    const jerry_value_t argc) {
    jerry_gc (JERRY_GC_PRESSURE_LOW);
    return jerry_create_undefined();
}

void InjectGlobalFunctions(void) {
    JerryValue global(jerry_get_global_object());


    if (Flag::Get(Flag::expose_gc)->u.bool_value) {
        JerryValue gc_string(jerry_create_string((const jerry_char_t*)"gc"));
        JerryValue gc_function(jerry_create_external_function(JerryHandlerGC));

        global.SetProperty(&gc_string, &gc_function);
    }
}


// TODO: remove these layering violations (this is a Jerry internal method, should not be visible here)
extern "C" bool ecma_get_object_is_builtin(void* obj);
#define ECMA_OBJECT_REF_ONE (1u << 6)
#define ECMA_OBJECT_MAX_REF (0x3ffu << 6)
#define ECMA_VALUE_TYPE_MASK 0x7u
typedef uint32_t ecma_value_t;

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
  void *ptr = (void *) (uintptr_t) ((value) & ~ECMA_VALUE_TYPE_MASK);
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
