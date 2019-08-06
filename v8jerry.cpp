#include <v8.h>

#include <libplatform/libplatform.h>

#include <cstring>

#include <stack>

#include "jerryscript.h"
#include "jerryscript-port-default.h"


class JerryIsolate;
class JerryHandleScope;
class JerryValue;
class JerryContext;

class JerryHandle {
public:
    enum Type {
        Context,
        Value
    };

    JerryHandle(Type type)
        : m_type(type)
    {}

    Type type() const { return m_type; }

private:
    Type m_type;
};

class JerryValue : public JerryHandle {
public:
    JerryValue(jerry_value_t value)
        : JerryHandle(JerryHandle::Value)
        , m_value(value)
    {}

    ~JerryValue(void) {
        if (m_value) {
            jerry_release_value(m_value);
        }
    }

    //jerry_value_t value() { return m_value; }
    jerry_value_t value() const { return m_value; }

    bool IsString() { return jerry_value_is_string(m_value); }

private:
    jerry_value_t m_value;
};

class JerryPlatform : public v8::Platform {
public:
    virtual void CallOnBackgroundThread(v8::Task*, v8::Platform::ExpectedRuntime) {}
    virtual void CallOnForegroundThread(v8::Isolate*, v8::Task*) {}
    virtual void CallDelayedOnForegroundThread(v8::Isolate*, v8::Task*, double) {}
    virtual double MonotonicallyIncreasingTime() { return 0.0; }
    virtual v8::TracingController* GetTracingController() { return NULL; }
};

class JerryHandleScope {
public:
    JerryHandleScope() {}

    ~JerryHandleScope();

    void AddHandle(JerryHandle* jvalue) {
        m_handles.push_back(jvalue);
    }

private:
    std::vector<JerryHandle*> m_handles;
};

class JerryIsolate {
public:
    JerryIsolate(const v8::Isolate::CreateParams& params)
    {
        jerry_init(JERRY_INIT_EMPTY/* | JERRY_INIT_MEM_STATS*/);
        jerry_port_default_set_abort_on_fail(true);
    }

    void Enter(void) {
        JerryIsolate::s_currentIsolate = this;
    }

    void Exit(void) {
        JerryIsolate::s_currentIsolate = nullptr;
    }

    void Dispose(void) {
        jerry_cleanup();
    }

    void PushHandleScope(void* ptr) {
        m_handleScopes.push(new JerryHandleScope());
    }

    void PopHandleScope(void* ptr) {
        JerryHandleScope* handleScope = m_handleScopes.top();
        m_handleScopes.pop();

        delete handleScope;
    }

    void AddToHandleScope(JerryHandle* jvalue) {
        m_handleScopes.top()->AddHandle(jvalue);
    }


    static v8::Isolate* toV8(JerryIsolate* iso) {
        return reinterpret_cast<v8::Isolate*>(iso);
    }

    static JerryIsolate* fromV8(v8::Isolate* iso) {
        return reinterpret_cast<JerryIsolate*>(iso);
    }

    static JerryIsolate* fromV8(v8::internal::Isolate* iso) {
        return reinterpret_cast<JerryIsolate*>(iso);
    }

    static JerryIsolate* GetCurrent(void) {
        return JerryIsolate::s_currentIsolate;
    }

private:
    static JerryIsolate* s_currentIsolate;

    std::stack<JerryHandleScope*> m_handleScopes;
};

JerryIsolate* JerryIsolate::s_currentIsolate = nullptr;

class JerryContext : public JerryHandle {
public:

    JerryContext(JerryIsolate* iso)
        : JerryHandle(JerryHandle::Context)
        , m_isolate(iso)
    {}

    JerryIsolate* GetIsolate(void) {
        return m_isolate;
    }

    void Enter(void) {
        m_isolate->Enter();
    }

    void Exit(void) {
        m_isolate->Exit();
    }

    static v8::Context* toV8(JerryContext* ctx) {
        return reinterpret_cast<v8::Context*>(ctx);
    }

    static JerryContext* fromV8(v8::Context* ctx) {
        return reinterpret_cast<JerryContext*>(ctx);
    }

private:
    JerryIsolate* m_isolate;
};

JerryHandleScope::~JerryHandleScope(void) {
    for (JerryHandle* jhandle : m_handles) {
        switch (jhandle->type()) {
            case JerryHandle::Value: delete reinterpret_cast<JerryValue*>(jhandle); break;
            case JerryHandle::Context: delete reinterpret_cast<JerryContext*>(jhandle); break;
        }
    }
}

/* V8 API helpers */
#define RETURN_HANDLE(T, ISOLATE, HANDLE) \
do {                                                                    \
    JerryHandle *__handle = HANDLE;                                     \
    return v8::Local<T>::New(ISOLATE, reinterpret_cast<T*>(&__handle)); \
} while (0)


/* API Implementation */
namespace v8 {

namespace platform {

    Platform* CreateDefaultPlatform(
        int thread_pool_size,
        IdleTaskSupport idle_task_support,
        InProcessStackDumping in_process_stack_dumping,
        v8::TracingController* tracing_controller) {
        return new JerryPlatform();
    }

} // namespace platform

bool V8::InitializeICUDefaultLocation(const char* exec_path, const char* icu_data_file) {
    return true;
}

void V8::InitializeExternalStartupData(char const*) { }

void V8::InitializePlatform(v8::Platform*) { }

bool V8::Initialize() {
    return true;
}

bool V8::Dispose() {
    return true;
}

void V8::ToLocalEmpty() { }

void V8::ShutdownPlatform() { }

/* ArrayBuffer & Allocator */
void ArrayBuffer::Allocator::SetProtection(void* data, size_t length, Protection protection) { }

void* ArrayBuffer::Allocator::Reserve(size_t length) {
    return nullptr;
}

void ArrayBuffer::Allocator::Free(void* data, size_t length, AllocationMode mode) { }

ArrayBuffer::Allocator* ArrayBuffer::Allocator::NewDefaultAllocator() {
    return nullptr;
}

/* Isolate */
ResourceConstraints::ResourceConstraints() { }

Isolate* Isolate::New(const CreateParams& params) {
    return JerryIsolate::toV8(new JerryIsolate(params));
}

Isolate* GetCurrent() {
    return JerryIsolate::toV8(JerryIsolate::GetCurrent());
}

bool Isolate::IsDead() {
    return false;
}

void Isolate::Enter() {
    JerryIsolate::fromV8(this)->Enter();
}

void Isolate::Exit() {
    JerryIsolate::fromV8(this)->Exit();
}

void Isolate::Dispose() {
    JerryIsolate::fromV8(this)->Dispose();
}

/* Context */
Local<Context> Context::New(Isolate* isolate,
                            ExtensionConfiguration* extensions /*= NULL*/,
                            MaybeLocal<ObjectTemplate> global_template /*= MaybeLocal<ObjectTemplate>()*/,
                            MaybeLocal<Value> global_object /*= MaybeLocal<Value>()*/) {
    RETURN_HANDLE(Context, isolate, new JerryContext(JerryIsolate::fromV8(isolate)));
}

Isolate* Context::GetIsolate() {
    return JerryIsolate::toV8(JerryContext::fromV8(this)->GetIsolate());
}

void Context::Enter() {
    JerryContext::fromV8(this)->Enter();
}

void Context::Exit() {
    JerryContext::fromV8(this)->Exit();
}

/* HandleScope */
HandleScope::HandleScope(Isolate* isolate)
    : isolate_(reinterpret_cast<internal::Isolate*>(isolate))
{
    JerryIsolate::fromV8(isolate_)->PushHandleScope(this);
}

HandleScope::~HandleScope(void) {
    JerryIsolate::fromV8(isolate_)->PopHandleScope(this);
}

internal::Object** HandleScope::CreateHandle(internal::Isolate* isolate, internal::Object* value) {
    JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(value);
    reinterpret_cast<JerryIsolate*>(isolate)->AddToHandleScope(jhandle);
    return reinterpret_cast<internal::Object**>(jhandle);
}

/* String */
Local<String> String::NewFromUtf8(
    Isolate* isolate, const char* data, NewStringType type /*= kNormalString*/, int length /*= -1*/) {
    if (length >= String::kMaxLength) {
        return Local<String>();
    }

    if (length == -1) {
        length = strlen(data);
    }

    jerry_value_t str_value = jerry_create_string_sz_from_utf8((const jerry_char_t*)data, length);
    RETURN_HANDLE(String, isolate, new JerryValue(str_value));
}

/** Allocates a new string from UTF-8 data. Only returns an empty value when
 * length > kMaxLength. **/
MaybeLocal<String> String::NewFromUtf8(
    Isolate* isolate, const char* data, v8::NewStringType type, int length /*= -1*/) {
    return String::NewFromUtf8(isolate, data, (String::NewStringType)type, length);
}

int String::WriteUtf8(char* buffer, int length, int* nchars_ref, int options) const {
    const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

    jerry_size_t bytes = jerry_string_to_utf8_char_buffer (jvalue->value(), (jerry_char_t *)buffer, length);

    buffer[bytes] = '\0';
    return (int)bytes;
}

int String::Utf8Length() const {
    const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);
    return jerry_get_utf8_string_size(jvalue->value());
}

String::Utf8Value::Utf8Value(Isolate*, Local<v8::Value> v8Value)
    : str_(nullptr)
    , length_(0)
{
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(*v8Value);

    jerry_value_t value;
    if (!jvalue->IsString()) {
        value = jerry_value_to_string(jvalue->value());
    } else {
        value = jvalue->value();
    }

    length_ = jerry_get_utf8_string_length(value);
    uint32_t size = (uint32_t)jerry_get_utf8_string_size(value);

    str_ = new char[size + 1];

    jerry_string_to_utf8_char_buffer (value, (jerry_char_t *)str_, size + 1);
    str_[size] = '\0';

    if (!jvalue->IsString()) {
        jerry_release_value(value);
    }
}

String::Utf8Value::~Utf8Value() {
    delete str_;
}

/* Script */
MaybeLocal<Script> Script::Compile(Local<Context> context, Local<String> source, ScriptOrigin* origin /* = nullptr */) {
    jerry_char_t* sourceString = new jerry_char_t[source->Utf8Length()];
    source->WriteUtf8((char*)sourceString, source->Utf8Length(), 0, 0);

    jerry_value_t scriptFunction = jerry_parse(NULL, 0, sourceString, source->Utf8Length(), JERRY_PARSE_NO_OPTS);

    delete sourceString;

    RETURN_HANDLE(Script, context->GetIsolate(), new JerryValue(scriptFunction));
}

MaybeLocal<Value> Script::Run(v8::Local<v8::Context> context) {
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(this);

    jerry_value_t result = jerry_run(jvalue->value());

    RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(result));
}

} // namespace v8
