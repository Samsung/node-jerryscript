#include "v8jerry_isolate.hpp"

#include <cassert>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "v8jerry_value.hpp"
#include "v8jerry_handlescope.hpp"
#include "v8jerry_templates.hpp"

JerryIsolate* JerryIsolate::s_currentIsolate = nullptr;

JerryIsolate::JerryIsolate(const v8::Isolate::CreateParams& params) {
    m_terminated = false;
    jerry_init(JERRY_INIT_EMPTY/* | JERRY_INIT_MEM_STATS*/);
    m_fatalErrorCallback = nullptr;

    m_fn_map_new = new JerryPolyfill("new_map", "", "return new Map();");
    m_fn_is_map = new JerryPolyfill("is_map", "value", "return value instanceof Map;");
    m_fn_is_set = new JerryPolyfill("is_set", "value", "return value instanceof Set;");
    m_fn_map_set = new JerryPolyfill("map_set", "map, key, value", "return map.set(key, value);");
    m_fn_object_assign = new JerryPolyfill("object_assign", "value", "return Object.assign({}, value);");
    m_fn_conversion_failer =
        new JerryPolyfill("conv_fail", "", "this.toString = this.valueOf = function() { throw new TypeError('Invalid usage'); }");
    m_fn_get_own_prop = new JerryPolyfill("get_own_prop", "key", "return Object.getOwnPropertyDescriptor(this, key);");
    m_fn_get_own_names = new JerryPolyfill("get_own_names", "", "return Object.getOwnPropertyNames(this);");
    m_fn_get_names = new JerryPolyfill("get_names", "", "var names = []; for (var name in this) { names.push(name); } return names;");

    InitalizeSlots();

    m_magic_string_stack = new JerryValue(jerry_create_string((const jerry_char_t*) "stack"));
    m_try_catch_count = 0;
    m_current_error = NULL;
    m_hidden_object_template = NULL;

#ifdef __POSIX__
    m_lock = PTHREAD_MUTEX_INITIALIZER;
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

namespace v8 {
    namespace internal {
        class Heap {
        public:
            static void DisposeExternalString(v8::String::ExternalStringResourceBase* external_string) {
                external_string->Dispose();
            }
        };
    }
}

void JerryIsolate::RunWeakCleanup(void) {
    for (std::vector<JerryValue*>::reverse_iterator it = m_weakrefs.rbegin();
        it != m_weakrefs.rend();
        it++) {
        // The weak callback will delete the JerryValue*
        (*it)->RunWeakCleanup();
    }
    m_weakrefs.clear();
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

    for (std::vector<v8::String::ExternalStringResource*>::iterator it = m_ext_str_res.begin();
        it != m_ext_str_res.end();
        it++) {
        v8::internal::Heap::DisposeExternalString(*it);
    }

    delete m_magic_string_stack;
    ClearError();

#ifdef __POSIX__
    pthread_mutex_destroy(&m_lock);
#endif

    delete m_fn_map_new;
    delete m_fn_is_map;
    delete m_fn_is_set;
    delete m_fn_map_set;
    delete m_fn_object_assign;
    delete m_fn_conversion_failer;
    delete m_fn_get_own_prop;
    delete m_fn_get_own_names;
    delete m_fn_get_names;

    // Release slots
    {
        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiPointerSize;
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex]);
        delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex]);
    }

    if (m_hidden_object_template != NULL) {
        delete m_hidden_object_template;
    }

    JerryForceCleanup();

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

void JerryIsolate::EnqueueMicrotask(v8::MicrotaskCallback callback, void* data) {
    m_tasks.push_back(new Task{callback, data});
}

void JerryIsolate::RunMicrotasks(void) {
    for (Task* task : m_tasks) {
        task->callback(task->data);
    }
    m_tasks.clear();

    // TODO: is it ok to have this here?
    jerry_run_all_enqueued_jobs();
}

void JerryIsolate::SetEternal(JerryValue* value, int* index) {
    if (*index == -1) {
        *index = m_eternals.size();
        m_eternals.push_back(value);
    } else {
        m_eternals[*index] = value;
    }
}

bool JerryIsolate::HasEternal(JerryValue* value) {
    return std::find(m_eternals.begin(), m_eternals.end(), value) != m_eternals.end();
}

bool JerryIsolate::HasAsWeak(JerryValue* value) {
    std::vector<JerryValue*>::iterator iter = std::find(m_weakrefs.begin(), m_weakrefs.end(), value);

    return iter != m_weakrefs.end();
}

void JerryIsolate::AddAsWeak(JerryValue* value) {
    assert(HasAsWeak(value) == false);

    std::vector<JerryValue*>::iterator iter = std::find(m_eternals.begin(), m_eternals.end(), value);
    // Just eternal objects can have weak reference.
    //assert(iter != m_eternals.end());
    if (iter != m_eternals.end()) {
        m_eternals.erase(iter);
    }

    m_weakrefs.push_back(value);
}

void JerryIsolate::RemoveAsWeak(JerryValue* value) {
    std::vector<JerryValue*>::iterator eternal_iter = std::find(m_eternals.begin(), m_eternals.end(), value);
    std::vector<JerryValue*>::iterator weak_iter = std::find(m_weakrefs.begin(), m_weakrefs.end(), value);

    assert(eternal_iter == m_eternals.end());
    assert(weak_iter != m_weakrefs.end());

    m_weakrefs.erase(weak_iter);
    m_eternals.push_back(value);
}

void JerryIsolate::AddExternalStringResource(v8::String::ExternalStringResource* resource) {
    std::vector<v8::String::ExternalStringResource*>::iterator iter = std::find(m_ext_str_res.begin(), m_ext_str_res.end(), resource);

    if (iter != m_ext_str_res.end()) {
        return;
    }

    m_ext_str_res.push_back(resource);
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
