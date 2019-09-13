#include "v8jerry_isolate.hpp"

#include <cassert>
#include <algorithm>

#include "v8jerry_value.hpp"
#include "v8jerry_handlescope.hpp"
#include "v8jerry_templates.hpp"

JerryIsolate* JerryIsolate::s_currentIsolate = nullptr;

JerryIsolate::JerryIsolate(const v8::Isolate::CreateParams& params) {
    jerry_init(JERRY_INIT_EMPTY/* | JERRY_INIT_MEM_STATS*/);
    jerry_port_default_set_abort_on_fail(true);
    m_fatalErrorCallback = nullptr;

    {/* new Map() method */
        const char* fn_args = "";
        const char* fn_body = "return new Map();";
        m_fn_map_new = new JerryValue(BuildHelperMethod(fn_args, fn_body));
    }
    {/* isMap helper method */
        const char* fn_args = "value";
        const char* fn_body = "return value instanceof Map;";
        m_fn_is_map = new JerryValue(BuildHelperMethod(fn_args, fn_body));
    }
    {/* isSet helper method */
        const char* fn_args = "value";
        const char* fn_body = "return value instanceof Set;";
        m_fn_is_set = new JerryValue(BuildHelperMethod(fn_args, fn_body));
    }
    {/* Map.Set helper method */
        const char* fn_args = "map, key, value";
        const char* fn_body = "return map.set(key, value);";

        m_fn_map_set = new JerryValue(BuildHelperMethod(fn_args, fn_body));
    }

    {/* Object.assing helper method */
        const char* fn_args = "value";
        const char* fn_body = "return Object.assing({}, value);";

        m_fn_object_assign = new JerryValue(BuildHelperMethod(fn_args, fn_body));
    }

    InitalizeSlots();
}

void JerryIsolate::Enter(void) {
    JerryIsolate::s_currentIsolate = this;
}

void JerryIsolate::Exit(void) {
    JerryIsolate::s_currentIsolate = nullptr;
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

    delete m_fn_map_new;
    delete m_fn_is_map;
    delete m_fn_is_set;
    delete m_fn_map_set;
    delete m_fn_object_assign;

    // Release slots
    {
        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiPointerSize;
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex]);
    }

    jerry_cleanup();

    // Warning!... Do not use the JerryIsolate after this!
    // If you do: dragons will spawn from the depths of the earth and tear everything apart!
    // You have been warned!
    delete this;
}

void JerryIsolate::PushContext(JerryContext* context) {
    // Contexts are managed by HandleScopes, here we only need the stack to correctly
    // return the current context if needed.
    m_contexts.push_back(context);
}

void JerryIsolate::PopContext(JerryContext* context) {
    JerryContext* ctx = m_contexts.back();
    assert(ctx == context);

    m_contexts.pop_back();
}

JerryContext* JerryIsolate::CurrentContext(void) {
    return m_contexts.back();
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
    (*end)->RemoveHandle(jvalue);
    ++end; // Step to a parent handleScope
    (*end)->AddHandle(jvalue);
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

void JerryIsolate::ReportFatalError(const char* location, const char* message) {
    if (m_fatalErrorCallback != nullptr) {
        m_fatalErrorCallback(location, message);
    }
}

JerryIsolate* JerryIsolate::GetCurrent(void) {
    return JerryIsolate::s_currentIsolate;
}

void JerryIsolate::InitalizeSlots(void) {
    ::memset(m_slot, 0, sizeof(m_slot));

    int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiPointerSize;
    m_slot[v8::internal::Internals::kExternalMemoryOffset / v8::internal::kApiPointerSize] = (void*) 0;
    m_slot[v8::internal::Internals::kExternalMemoryLimitOffset / v8::internal::kApiPointerSize] = (void*) (1024*1024);
                // v8::internal::kExternalAllocationSoftLimit

    // Undefined
    m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex] = new JerryValue(jerry_create_undefined());
    // Null
    m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex] = new JerryValue(jerry_create_null());
    // Boolean True
    m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex] = new JerryValue(jerry_create_boolean(true));
    // Boolean False
    m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex] = new JerryValue(jerry_create_boolean(false));
    // Empty string
    m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex] = new JerryValue(jerry_create_string_sz((const jerry_char_t*)"", 0));
    // TODO: do we need these?
    //m_slot[root_offset + v8::internal::Internals::kInt32ReturnValuePlaceholderIndex] =
    //m_slot[root_offset + v8::internal::Internals::kUint32ReturnValuePlaceholderIndex] =
    //m_slot[root_offset + v8::internal::Internals::kDoubleReturnValuePlaceholderIndex] =
}

