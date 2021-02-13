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

JerryIsolate* JerryIsolate::s_currentIsolate = nullptr;

JerryIsolate::JerryIsolate() {
}

JerryIsolate::JerryIsolate(const v8::Isolate::CreateParams& params) {
    this->InitializeJerryIsolate(params);
}

static void error_object_created_callback(const jerry_value_t error_object, void *user_p) {
    (void) user_p;
    CreateStackTrace(error_object, NULL);
}

void JerryIsolate::InitializeJerryIsolate(const v8::Isolate::CreateParams& params) {
    m_terminated = false;
    jerry_init_flag_t flag = (jerry_init_flag_t) JERRY_INIT_EMPTY;
#if (defined JERRY_MEM_STATS && JERRY_MEM_STATS)
    jerry_port_default_set_log_level (JERRY_LOG_LEVEL_DEBUG);
    flag = (jerry_init_flag_t) (JERRY_INIT_MEM_STATS | JERRY_INIT_EMPTY);
#endif
    jerry_init(flag);

    m_fatalErrorCallback = NULL;
    m_messageCallback = NULL;
    m_prepare_stack_trace_callback = NULL;

    m_fn_map_set = new JerryPolyfill("map_set", "map, key, value", "return map.set(key, value);");
    m_fn_set_add = new JerryPolyfill("set_add", "set, value", "return map.add(key);");
    m_fn_object_assign = new JerryPolyfill("object_assign", "value", "return Object.assign(Array.isArray(value) ? [] : {}, value);");
    m_fn_conversion_failer =
        new JerryPolyfill("conv_fail", "", "this.toString = this.valueOf = function() { throw new TypeError('Invalid usage'); }");
    m_fn_get_own_prop = new JerryPolyfill("get_own_prop", "key", "return Object.getOwnPropertyDescriptor(this, key);");
    m_fn_get_own_names = new JerryPolyfill("get_own_names", "", "return Object.getOwnPropertyNames(this);");
    m_fn_set_integrity = new JerryPolyfill("set_integrity", "prop", "Object[prop](this)");

    InitalizeSlots();

    m_magic_string_stack = new JerryValue(jerry_create_string((const jerry_char_t*) "stack"));

    char *call_site_prototype = "({"
                                "  getLineNumber() { return this.line__ },"
                                "  getColumnNumber() { return 1 },"
                                "  getFileName() { return this.resource__ },"
                                "  getFunctionName() { return (this.function__ && this.function__.name) ? new String(this.function__.name) : null },"
                                "  toString() { return this.resource__ + ':' + this.line__ + ':1' },"
                                "})";
    m_call_site_prototype = new JerryValue(jerry_eval((jerry_char_t*)call_site_prototype, strlen(call_site_prototype), 0));

    m_last_handle_scope = NULL;
    m_last_try_catch = NULL;
    m_has_current_error = false;
    m_current_error = jerry_create_undefined();
    m_global_error = NULL;
    m_try_depth = 0;
    m_promise_hook = NULL;
    m_hidden_object_template = NULL;

    // Initialize random for math functions
    srand((unsigned)time(NULL));

#ifdef __POSIX__
    m_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

    InjectGlobalFunctions();

    jerry_set_error_object_created_callback (error_object_created_callback, NULL);

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

    if (HasError()) {
        jerry_release_value(GetError());
    }

    delete m_magic_string_stack;
    delete m_call_site_prototype;

#ifdef __POSIX__
    pthread_mutex_destroy(&m_lock);
#endif

    delete m_fn_set_add;
    delete m_fn_map_set;
    delete m_fn_object_assign;
    delete m_fn_conversion_failer;
    delete m_fn_get_own_prop;
    delete m_fn_get_own_names;
    delete m_fn_set_integrity;

    // Release slots
    {
        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiSystemPointerSize;
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kTheHoleValueRootIndex]);
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

void JerryIsolate::SetError(const jerry_value_t error_value) {
    assert(jerry_value_is_error(error_value));

    if (HasError()) {
        jerry_release_value(GetError());
    }

    m_has_current_error = true;
    m_current_error = jerry_get_value_from_error(error_value, true);
}

void JerryIsolate::RestoreError(JerryValue* error) {
    if (error == NULL) {
        return;
    }

    if (HasError()) {
        jerry_release_value(GetError());
    }

    m_has_current_error = true;
    m_current_error = error->value();

    JerryValue::DeleteValueWithoutRelease(error);
}

void JerryIsolate::ProcessError(bool force) {
    if (!force && (m_last_try_catch != NULL || m_try_depth > 0)) {
        return;
    }

    if (Flag::Get(Flag::abort_on_uncaught_exception)->u.bool_value
            && (m_abortOnUncaughtExceptionCallback == NULL || m_abortOnUncaughtExceptionCallback(toV8(this)))) {
        abort();
    }

    // Move the error
    JerryValue* current_error = new JerryValue(TakeError());
    v8::Local<v8::Value> exception(current_error->AsLocal<v8::Value>());
    v8::Local<v8::Message> message;

    ReportMessage(message, exception);

    delete current_error;

    if (HasError()) {
        jerry_release_value(TakeError());
    }
}

void JerryIsolate::PushContext(JerryValue* context) {
    // Contexts are managed by HandleScopes, here we only need the stack to correctly
    // return the current context if needed.
    jerry_value_t old_realm = jerry_set_realm(context->value());
    m_contexts.push_back(std::pair<JerryValue*, jerry_value_t>(context, old_realm));
    JerryIsolate::s_currentIsolate = this;
}

void JerryIsolate::PopContext() {
    jerry_set_realm(m_contexts.back().second);
    m_contexts.pop_back();
}

JerryValue* JerryIsolate::CurrentContext(void) {
    return m_contexts.back().first;
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

void JerryIsolate::PushHandleScope(JerryHandleScopeType type) {
    m_last_handle_scope = new JerryHandleScope(type, m_last_handle_scope);
}

void JerryIsolate::PopHandleScope() {
    JerryHandleScope* handle_scope = m_last_handle_scope;

    m_last_handle_scope = m_last_handle_scope->PrevHandleScope();
    handle_scope->FreeHandles(*GetLastReturnValue());
    delete handle_scope;
}

JerryHandleScope* JerryIsolate::CurrentHandleScope(void) {
    return m_last_handle_scope;
}

void JerryIsolate::AddToHandleScope(JerryValue* jvalue) {
    m_last_handle_scope->AddHandle(jvalue);
}

void JerryIsolate::EscapeHandle(JerryValue* jvalue) {
    assert(m_last_handle_scope->PrevHandleScope() != NULL);

    // If the handle was removed simply add it to the parent handle scope.
    // However if it was not in the current handle scope (was not removed)
    // then the value is a refernece to an enternal element, thus there is nothing to do.
    if (m_last_handle_scope->RemoveHandle(jvalue)) {
        m_last_handle_scope->PrevHandleScope()->AddHandle(jvalue);
    }
}

void JerryIsolate::SealHandleScope(void* handle_scope) {
    reinterpret_cast<JerryHandleScope*>(handle_scope)->Seal();
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
    // Hole
    m_slot[root_offset + v8::internal::Internals::kTheHoleValueRootIndex] = new JerryValue(jerry_create_undefined(), JerryHandle::PersistentValue);
    // Null
    m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex] = new JerryValue(jerry_create_null(), JerryHandle::PersistentValue);
    // Boolean True
    m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex] = new JerryValue(jerry_create_boolean(true), JerryHandle::PersistentValue);
    // Boolean False
    m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex] = new JerryValue(jerry_create_boolean(false), JerryHandle::PersistentValue);
    // Empty string
    m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex] = new JerryValue(jerry_create_string_sz((const jerry_char_t*)"", 0), JerryHandle::PersistentValue);

    // TODO: do we need these?
    //m_slot[root_offset + v8::internal::Internals::kInt32ReturnValuePlaceholderIndex] =
    //m_slot[root_offset + v8::internal::Internals::kUint32ReturnValuePlaceholderIndex] =
    //m_slot[root_offset + v8::internal::Internals::kDoubleReturnValuePlaceholderIndex] =

    m_last_return_value = (JerryValue**) &m_slot[root_offset + v8::internal::Internals::kTheHoleValueRootIndex];

    assert((sizeof(m_slot) / sizeof(m_slot[0])) > root_offset + v8::internal::Internals::kEmptyStringRootIndex);
}

void JerryIsolate::EnqueueMicrotask(JerryValue *func) {
    m_micro_tasks.push_back(func->Copy());
}

void JerryIsolate::RunMicrotasks(void) {
    for (auto& task : m_micro_tasks) {
        jerry_release_value(jerry_call_function (task->value(), jerry_create_undefined(), NULL, 0));
        delete task;
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

JerryObjectTemplate* JerryIsolate::HiddenObjectTemplate(void) {
    if (m_hidden_object_template == NULL) {
        m_hidden_object_template = new JerryObjectTemplate();
    }

    return m_hidden_object_template;
}

struct PromiseData {
    jerry_value_t await_promise;
    jerry_value_t job_promise;
};

void PromiseDataFree(void *native_p) {
    PromiseData* data = reinterpret_cast<PromiseData*>(native_p);
    jerry_release_value(data->await_promise);
    jerry_release_value(data->job_promise);
    delete data;
}

static jerry_object_native_info_t PromiseDataTypeInfo = {
    .free_cb = PromiseDataFree,
};

static void promise_callback (jerry_promise_event_type_t event_type,
                              const jerry_value_t object, const jerry_value_t value, void *user_p) {
    JerryIsolate* isolate = reinterpret_cast<JerryIsolate*>(user_p);
    v8::PromiseHook promise_hook = isolate->GetPromiseHook();

    if (promise_hook == NULL) {
        return;
    }

    v8::HandleScope handle_scope(JerryIsolate::toV8(isolate));

    switch (event_type) {
    case JERRY_PROMISE_EVENT_CREATE: {
        JerryValueNoRelease promise(object);
        JerryValueNoRelease parent(value);

        promise_hook(v8::PromiseHookType::kInit, promise.AsLocal<v8::Promise>(), parent.AsLocal<v8::Value>());
        return;
    }
    case JERRY_PROMISE_EVENT_RESOLVE:
    case JERRY_PROMISE_EVENT_REJECT:
    case JERRY_PROMISE_EVENT_BEFORE_REACTION_JOB:
    case JERRY_PROMISE_EVENT_AFTER_REACTION_JOB: {
        v8::PromiseHookType type = v8::PromiseHookType::kResolve;

        if (event_type == JERRY_PROMISE_EVENT_BEFORE_REACTION_JOB) {
            type = v8::PromiseHookType::kBefore;
        } else if (event_type == JERRY_PROMISE_EVENT_AFTER_REACTION_JOB) {
            type = v8::PromiseHookType::kAfter;
        }

        JerryValueNoRelease promise(object);
        promise_hook(type, promise.AsLocal<v8::Promise>(), isolate->Undefined()->AsLocal<v8::Value>());
        return;
    }
    case JERRY_PROMISE_EVENT_ASYNC_AWAIT: {
        PromiseData *data;

        if (!jerry_get_object_native_pointer (object, reinterpret_cast<void**>(&data), &PromiseDataTypeInfo)) {
            data = new PromiseData;

            data->job_promise = jerry_create_undefined();

            jerry_set_object_native_pointer(object, reinterpret_cast<void*>(data), &PromiseDataTypeInfo);
        }

        jerry_promise_set_callback (NULL, NULL);
        JerryValueNoRelease promise(jerry_create_promise());
        jerry_promise_set_callback (promise_callback, reinterpret_cast<void*>(isolate));

        data->await_promise = promise.value();

        JerryValueNoRelease parent(value);
        promise_hook(v8::PromiseHookType::kInit, promise.AsLocal<v8::Promise>(), parent.AsLocal<v8::Value>());
        return;
    }
    case JERRY_PROMISE_EVENT_ASYNC_BEFORE_RESOLVE:
    case JERRY_PROMISE_EVENT_ASYNC_BEFORE_REJECT: {
        PromiseData *data;

        if (!jerry_get_object_native_pointer (object, reinterpret_cast<void**>(&data), &PromiseDataTypeInfo)) {
            return;
        }

        JerryValueNoRelease promise(data->await_promise);

        data->job_promise = promise.value();
        data->await_promise = jerry_create_undefined();

        promise_hook(v8::PromiseHookType::kBefore, promise.AsLocal<v8::Promise>(), isolate->Undefined()->AsLocal<v8::Value>());
        return;
    }
    case JERRY_PROMISE_EVENT_ASYNC_AFTER_RESOLVE:
    case JERRY_PROMISE_EVENT_ASYNC_AFTER_REJECT: {
        PromiseData *data;

        if (!jerry_get_object_native_pointer (object, reinterpret_cast<void**>(&data), &PromiseDataTypeInfo)) {
            return;
        }

        JerryValue promise(data->job_promise);
        data->job_promise = jerry_create_undefined();

        promise_hook(v8::PromiseHookType::kResolve, promise.AsLocal<v8::Promise>(), isolate->Undefined()->AsLocal<v8::Value>());
        promise_hook(v8::PromiseHookType::kAfter, promise.AsLocal<v8::Promise>(), isolate->Undefined()->AsLocal<v8::Value>());
        return;
    }
    }
}

void JerryIsolate::SetPromiseHook(v8::PromiseHook promise_hook) {
    m_promise_hook = promise_hook;

    if (m_promise_hook != NULL) {
        jerry_promise_set_callback (promise_callback, reinterpret_cast<void*>(this));
    } else {
        jerry_promise_set_callback (NULL, NULL);
    }
}
