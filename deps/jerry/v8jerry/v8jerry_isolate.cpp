#include "v8jerry_isolate.hpp"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "jerryscript-ext/debugger.h"

#include "v8jerry_handlescope.hpp"
#include "v8jerry_flags.hpp"
#include "v8jerry_templates.hpp"
#include "v8jerry_utils.hpp"

#define DEBUG_PRINT 1

JerryIsolate* JerryIsolate::s_currentIsolate = nullptr;

JerryIsolate::JerryIsolate() {
}

JerryIsolate::JerryIsolate(const v8::Isolate::CreateParams& params) {
    this->InitializeJerryIsolate(params);
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

static void jerryx_handler_register (const jerry_char_t *name_p, jerry_value_t object_value,
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

static void jerryx_handler_register_global (const jerry_char_t *name_p,
                                            jerry_external_handler_t handler_p) {
  jerry_value_t global_obj_val = jerry_get_global_object();
  jerryx_handler_register(name_p, global_obj_val, handler_p);
  jerry_release_value(global_obj_val);
}

static void jerryx_handler_register_string (const jerry_char_t *name_p,
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

  jerryx_handler_register (name_p, prototype_val, handler_p);
  jerry_release_value (prototype_val);
}

#endif

void JerryIsolate::InitializeJerryIsolate(const v8::Isolate::CreateParams& params) {
    m_terminated = false;
    jerry_init(JERRY_INIT_EMPTY/* | JERRY_INIT_MEM_STATS*/);
#if DEBUG_PRINT
    jerryx_handler_register_global((const jerry_char_t *)"print", jerryx_handler_print);
#endif
    jerryx_handler_register_string((const jerry_char_t *)"normalize", jerryx_handler_string_normalize);

    m_fatalErrorCallback = nullptr;

    m_fn_map_set = new JerryPolyfill("map_set", "map, key, value", "return map.set(key, value);");
    m_fn_set_add = new JerryPolyfill("set_add", "set, value", "return map.add(key);");
    m_fn_object_assign = new JerryPolyfill("object_assign", "value", "return Object.assign(Array.isArray(value) ? [] : {}, value);");
    m_fn_conversion_failer =
        new JerryPolyfill("conv_fail", "", "this.toString = this.valueOf = function() { throw new TypeError('Invalid usage'); }");
    m_fn_get_own_prop = new JerryPolyfill("get_own_prop", "key", "return Object.getOwnPropertyDescriptor(this, key);");
    m_fn_get_own_names = new JerryPolyfill("get_own_names", "", "return Object.getOwnPropertyNames(this);");
    m_fn_get_names = new JerryPolyfill("get_names", "", "var names = []; for (var name in this) { names.push(name); } return names;");
    m_fn_set_integrity = new JerryPolyfill("set_integrity", "prop", "Object[prop](this)");

    InitalizeSlots();
    InitializeSharedArrayBuffer();
    InitializeAtomics();

    m_magic_string_stack = new JerryValue(jerry_create_string((const jerry_char_t*) "stack"));
    m_try_catch_count = 0;
    m_current_error = NULL;
    m_hidden_object_template = NULL;

#ifdef __POSIX__
    m_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

    InjectGlobalFunctions();

#if (defined JERRY_DEBUGGER && JERRY_DEBUGGER)
    bool protocol = jerryx_debugger_tcp_create (5001);
    jerryx_debugger_after_connect (protocol && jerryx_debugger_ws_create ());
#endif
}


void JerryIsolate::Enter(void) {
    JerryIsolate::s_currentIsolate = this;
}

void JerryIsolate::Exit(void) {
    if (m_contexts.size() == 0) {
        JerryIsolate::s_currentIsolate = NULL;
    }
}

static jerry_value_t IsolateTerminateCallback(void *user_p) {
    return jerry_create_string ((const jerry_char_t *) "Script Abort Requested");
}

void JerryIsolate::Terminate(void) {
    m_terminated = true;

    jerry_set_vm_exec_stop_callback(IsolateTerminateCallback, NULL, 1);
}

void JerryIsolate::CancelTerminate(void) {
    m_terminated = false;
    jerry_set_vm_exec_stop_callback(NULL, NULL, 1);
}

void JerryIsolate::Dispose(void) {
    for (std::vector<JerryTemplate*>::reverse_iterator it = m_templates.rbegin();
        it != m_templates.rend();
        it++) {
        JerryHandle* jhandle = *it;
        switch (jhandle->type()) {
            case JerryHandle::FunctionTemplate: delete reinterpret_cast<JerryFunctionTemplate*>(jhandle); break;
            case JerryHandle::ObjectTemplate: delete reinterpret_cast<JerryObjectTemplate*>(jhandle); break;
            default:
                fprintf(stderr, "Isolate::Dispose unsupported type (%d)\n", jhandle->type());
                break;
        }
    }

    for (std::vector<JerryValue*>::iterator it = m_eternals.begin();
        it != m_eternals.end();
        it++) {
        delete *it;
    }

    for (auto &it : m_global_symbols) {
        jerry_release_value (it.first);
        jerry_release_value (it.second);
    }

    delete m_magic_string_stack;
    ClearError();

#ifdef __POSIX__
    pthread_mutex_destroy(&m_lock);
#endif

    delete m_fn_set_add;
    delete m_fn_map_set;
    delete m_fn_object_assign;
    delete m_fn_conversion_failer;
    delete m_fn_get_own_prop;
    delete m_fn_get_own_names;
    delete m_fn_get_names;
    delete m_fn_set_integrity;

    // Release slots
    {
        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiSystemPointerSize;
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex]);
    }

    if (m_hidden_object_template != NULL) {
        delete m_hidden_object_template;
    }

    m_micro_tasks.clear();

    //JerryForceCleanup();

    jerry_cleanup();

    // Warning!... Do not use the JerryIsolate after this!
    // If you do: dragons will spawn from the depths of the earth and tear everything apart!
    // You have been warned!
    delete this;
}

void JerryIsolate::PushTryCatch(void* try_catch_obj) {
    m_try_catch_count++;
}

void JerryIsolate::PopTryCatch(void* try_catch_obj) {
    assert(m_try_catch_count > 0);

    m_try_catch_count--;
}

void JerryIsolate::SetError(JerryValue* error) {
    assert(error != NULL);
    // If there was a previous error, release it!
    ClearError();
    m_current_error = error;
}

void JerryIsolate::SetError(const jerry_value_t error_value) {
    assert(jerry_value_is_error(error_value));

    jerry_value_t error_obj = jerry_get_value_from_error(error_value, true);
    SetError(new JerryValue(error_obj));
}

void JerryIsolate::ClearError(void) {
    if (m_current_error != NULL) {
        delete m_current_error;
        m_current_error = NULL;
    }
}

bool JerryIsolate::HasError(void) {
    return m_current_error != NULL;
}

void JerryIsolate::TryReportError(void) {
    if (m_try_catch_count != 0) {
        return;
    }

    // Copy the error to corectly report error
    JerryValue error(jerry_acquire_value(GetRawError()->value()));

    ClearError();
    v8::Local<v8::Value> exception = error.AsLocal<v8::Value>();

    // Replace "stack" property on exception as V8 creates a stack string and not an array.
    UpdateErrorStackProp(error);

    v8::Local<v8::Message> message;
    ReportMessage(message, exception);
}

void JerryIsolate::PushContext(JerryValue* context) {
    // Contexts are managed by HandleScopes, here we only need the stack to correctly
    // return the current context if needed.
    m_contexts.push_back(context);

    JerryIsolate::s_currentIsolate = this;
}

void JerryIsolate::PopContext(JerryValue* context) {
    JerryValue* ctx = m_contexts.back();
    assert(ctx == context);

    m_contexts.pop_back();
}

JerryValue* JerryIsolate::CurrentContext(void) {
    return m_contexts.back();
}

JerryValue* JerryIsolate::GetGlobalSymbol(JerryValue* name) {
    jerry_value_t nameValue = name->value();
    for (auto &it : m_global_symbols) {
        if (jerry_get_boolean_value (jerry_binary_operation(JERRY_BIN_OP_STRICT_EQUAL, it.first, nameValue))) {
            return new JerryValue(jerry_acquire_value (it.second));
        }
    }
    jerry_value_t symbol = jerry_create_symbol(nameValue);
    m_global_symbols.push_back(std::pair<jerry_value_t, jerry_value_t>(jerry_acquire_value (nameValue), jerry_acquire_value (symbol)));

    return new JerryValue(symbol);
}

void JerryIsolate::PushHandleScope(JerryHandleScopeType type, void* handle_scope) {
    m_handleScopes.push_back(new JerryHandleScope(type, handle_scope));
}

void JerryIsolate::PopHandleScope(void* handle_scope) {
    JerryHandleScope* handleScope = m_handleScopes.back();

    assert(handleScope->V8HandleScope() == handle_scope);

    m_handleScopes.pop_back();

    delete handleScope;
}

JerryHandleScope* JerryIsolate::CurrentHandleScope(void) {
    return m_handleScopes.back();
}

void JerryIsolate::AddToHandleScope(JerryHandle* jvalue) {
    m_handleScopes.back()->AddHandle(jvalue);
}

void JerryIsolate::EscapeHandle(JerryHandle* jvalue) {
    assert(m_handleScopes.size() > 1);

    std::deque<JerryHandleScope*>::reverse_iterator end = m_handleScopes.rbegin();
    bool was_removed = (*end)->RemoveHandle(jvalue);
    // If the handle was removed simply add it to the parent handle scope.
    // However if it was not in the current handle scope (was not removed)
    // then the value is a refernece to an enternal element, thus there is nothing to do.
    if (was_removed) {
        ++end; // Step to a parent handleScope
        (*end)->AddHandle(jvalue);
    }
}

void JerryIsolate::SealHandleScope(void* handle_scope) {
    assert(m_handleScopes.back()->V8HandleScope() == handle_scope);
    m_handleScopes.back()->Seal();
}

void JerryIsolate::AddTemplate(JerryTemplate* handle) {
    // TODO: make the vector a set or a map
    if (std::find(std::begin(m_templates), std::end(m_templates), handle) == std::end(m_templates)) {
        m_templates.push_back(handle);
    }
}

void JerryIsolate::ReportMessage(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
    if (m_messageCallback != NULL) {
        m_messageCallback(message, error);
    }
}

void JerryIsolate::ReportFatalError(const char* location, const char* message) {
    if (m_fatalErrorCallback != nullptr) {
        m_fatalErrorCallback(location, message);
    } else {
        std::cerr << "Fatal error: " << location << " " << message << std::endl;
        abort();
    }
}

JerryIsolate* JerryIsolate::GetCurrent(void) {
    return JerryIsolate::s_currentIsolate;
}

void JerryIsolate::InitalizeSlots(void) {
    ::memset(m_slot, 0, sizeof(m_slot));

    int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiSystemPointerSize;
    m_slot[v8::internal::Internals::kExternalMemoryOffset / v8::internal::kApiSystemPointerSize] = (void*) 0;
    m_slot[v8::internal::Internals::kExternalMemoryLimitOffset / v8::internal::kApiSystemPointerSize] = (void*) (1024*1024);
                // v8::internal::kExternalAllocationSoftLimit

    // Undefined
    m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex] = new JerryValue(jerry_create_undefined(), JerryHandle::PersistentValue);
    // Null
    m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex] = new JerryValue(jerry_create_null(), JerryHandle::PersistentValue);
    // Boolean True
    m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex] = new JerryValue(jerry_create_boolean(true), JerryHandle::PersistentValue);
    // Boolean False
    m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex] = new JerryValue(jerry_create_boolean(false), JerryHandle::PersistentValue);
    // Empty string
    m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex] = new JerryValue(jerry_create_string_sz((const jerry_char_t*)"", 0), JerryHandle::PersistentValue);

    assert((sizeof(m_slot) / sizeof(m_slot[0])) > root_offset + v8::internal::Internals::kEmptyStringRootIndex);

    // TODO: do we need these?
    //m_slot[root_offset + v8::internal::Internals::kInt32ReturnValuePlaceholderIndex] =
    //m_slot[root_offset + v8::internal::Internals::kUint32ReturnValuePlaceholderIndex] =
    //m_slot[root_offset + v8::internal::Internals::kDoubleReturnValuePlaceholderIndex] =
}

static jerry_value_t shared_array_buffer_constructor(
    const jerry_value_t func_value, /**< function object */
    const jerry_value_t this_val,   /**< this arg */
    const jerry_value_t args_p[],   /**< function arguments */
    const jerry_length_t args_cnt)  /**< number of function arguments */
{
    if ((args_cnt == 0) || (jerry_value_is_number(args_p[0]) == false)) {
        return jerry_create_undefined();
    }

    jerry_length_t size = (jerry_length_t) jerry_get_number_value(args_p[0]);
    jerry_value_t array = jerry_create_arraybuffer(size);

    // TODO: Create mutex for memory sharing

    return array;
}

void JerryIsolate::InitializeSharedArrayBuffer() {
    jerryx_handler_register_global((const jerry_char_t*) "SharedArrayBuffer",
                                    shared_array_buffer_constructor);
    // TODO: Create map of mutexes for memory sharing
}

static jerry_value_t atomics_handler(
    const jerry_value_t func_value, /**< function object */
    const jerry_value_t this_val,   /**< this arg */
    const jerry_value_t args_p[],   /**< function arguments */
    const jerry_length_t args_cnt)  /**< number of function arguments */
{
    return jerry_create_undefined();
}

void JerryIsolate::InitializeAtomics() {
    jerryx_handler_register_global((const jerry_char_t*) "Atomics",
                                    atomics_handler);
    // TODO: Add handlers for Atomics methods
}

void JerryIsolate::EnqueueMicrotask(JerryValue *func) {
    m_micro_tasks.push_back(func->Copy());
}

void JerryIsolate::RunMicrotasks(void) {
    for (auto& task : m_micro_tasks) {
        jerry_value_t call_res = jerry_call_function (task->value(), jerry_create_undefined(), NULL, 0);
        jerry_release_value (call_res);
    }
    m_micro_tasks.clear();

    while (true) {
        jerry_value_t ret = jerry_run_all_enqueued_jobs();

        bool end = jerry_value_is_undefined(ret);
        jerry_release_value(ret);

        if (end) {
            break;
        }
    }

}

void JerryIsolate::SetEternal(JerryValue* value, int* index) {
    if (*index == -1) {
        *index = m_eternals.size();
        m_eternals.push_back(value);
    } else {
        m_eternals[*index] = value;
    }
}

bool JerryIsolate::IsEternal(JerryValue* value) {
    return std::find(m_eternals.begin(), m_eternals.end(), value) != m_eternals.end();
}

void JerryIsolate::AddUTF16String(std::u16string* str) {
    uint16_t* buffer = (uint16_t*) str->c_str();
    assert(m_utf16strs.find(buffer) == m_utf16strs.end());

    m_utf16strs[buffer] = str;
}

void JerryIsolate::RemoveUTF16String(uint16_t* buffer) {
    std::unordered_map<uint16_t*, std::u16string*>::iterator iter = m_utf16strs.find(buffer);

    assert(iter != m_utf16strs.end());

    delete iter->second;
    m_utf16strs.erase(iter);
}

void JerryIsolate::FormatError(const JerryValue& error, std::ostream& out) {
    {
        jerry_value_t errorStr = jerry_value_to_string(error.value());
        jerry_size_t msg_size = jerry_get_string_size(errorStr);

        std::vector<jerry_char_t> msg;
        msg.resize(msg_size + 1);

        jerry_size_t copied = jerry_string_to_char_buffer(errorStr, &msg[0], msg_size + 1);
        jerry_release_value(errorStr);

        assert(copied == msg_size);
        msg[copied] = static_cast<jerry_char_t>('\0');

        out << (const char*)&msg[0] << std::endl;
    }

    // No default handler print out trace
    jerry_value_t stack_trace = jerry_get_property(error.value(), m_magic_string_stack->value());

    assert(!jerry_value_is_error(stack_trace));

    uint32_t array_length = jerry_get_array_length(stack_trace);
    for (uint32_t idx = 0; idx < array_length; idx++)
    {
        jerry_value_t property = jerry_get_property_by_index(stack_trace, idx);

        jerry_size_t msg_size = jerry_get_string_size(property);

        std::vector<jerry_char_t> msg;
        msg.resize(msg_size + 1);

        jerry_size_t copied = jerry_string_to_char_buffer(property, &msg[0], msg_size + 1);
        assert(copied == msg_size);
        msg[copied] = static_cast<jerry_char_t>('\0');

        out << "# " << idx << ": " << (const char*)&msg[0] << std::endl;

        jerry_release_value(property);
    }

    jerry_release_value(stack_trace);
}

void JerryIsolate::UpdateErrorStackProp(JerryValue& error) {
    std::ostringstream errorMessage;
    FormatError(error, errorMessage);

    JerryValue newMessage(jerry_create_string((const jerry_char_t*)errorMessage.str().c_str()));
    error.SetProperty(m_magic_string_stack, &newMessage);
}

JerryObjectTemplate* JerryIsolate::HiddenObjectTemplate(void) {
    if (m_hidden_object_template == NULL) {
        m_hidden_object_template = new JerryObjectTemplate();
    }

    return m_hidden_object_template;
}
