#include <v8.h>

#include <libplatform/libplatform.h>

#include <cstring>
#include <stack>
#include <vector>
#include <algorithm>

#include "jerryscript.h"
#include "jerryscript-port-default.h"

/* Tooling helper methods*/

static jerry_value_t BuildHelperMethod(const char* fn_args, const char* fn_body) {
    jerry_value_t method = jerry_parse_function(NULL, 0,
                                 reinterpret_cast<const jerry_char_t*>(fn_args), strlen(fn_args),
                                 reinterpret_cast<const jerry_char_t*>(fn_body), strlen(fn_body),
                                 JERRY_PARSE_NO_OPTS);
    if (jerry_value_is_error(method)) {
        fprintf(stderr, "Failed to build helper method initialize at: %s:%d\nfunction (%s) {\n%s\n}", __FILE__, __LINE__, fn_args, fn_body);
        abort();
    }
    return method;
}

/* Jerry <-> V8 binding classes */
class JerryIsolate;
class JerryHandleScope;
class JerryValue;
class JerryContext;

class JerryHandle {
public:
    enum Type {
        Context,
        FunctionTemplate,
        ObjectTemplate,
        Value,
    };

    JerryHandle() {}

    JerryHandle(Type type)
        : m_type(type)
    {}

    Type type() const { return m_type; }

private:
    Type m_type;
};

static jerry_object_native_info_t JerryV8ExternalTypeInfo = {
    .free_cb = NULL,
};

class JerryValue : public JerryHandle {
public:
    JerryValue()
        : JerryHandle(JerryHandle::Value)
    {}

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

    bool SetProperty(JerryValue* key, JerryValue* value) {
        // TODO: NULL check assert for key, value
        jerry_value_t result = jerry_set_property(m_value, key->value(), value->value());
        bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
        jerry_release_value(result);

        return isOk;
    }

    bool SetPropertyIdx(uint32_t idx, JerryValue* value) {
        // TODO: NULL check assert for value
        jerry_value_t result = jerry_set_property_by_index(m_value, idx, value->value());
        bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
        jerry_release_value(result);

        return isOk;
    }

    bool IsString() const { return jerry_value_is_string(m_value); }
    bool IsBoolean() const { return jerry_value_is_boolean(m_value); }
    bool IsFalse() const { return jerry_value_is_boolean(m_value) && !jerry_get_boolean_value(m_value); }
    bool IsTrue() const { return jerry_value_is_boolean(m_value) && jerry_get_boolean_value(m_value); }
    bool IsPromise() const { return jerry_value_is_promise(m_value); }
    bool IsArray() const { return jerry_value_is_array(m_value); }
    bool IsNumber() const {
        return jerry_value_is_number(m_value) && ((int) jerry_get_number_value(m_value) != jerry_get_number_value(m_value));
    }
    bool IsUint32() const { return jerry_value_is_number(m_value); } /* Todo: fix it. */
    bool IsInt32() const { return jerry_value_is_number(m_value); } /* Todo: fix it. */
    bool IsFunction() const { return jerry_value_is_function(m_value); }
    bool IsObject() const { return jerry_value_is_object(m_value); }
    bool IsSymbol() const { return jerry_value_is_symbol(m_value); }
    bool IsTypedArray() const { return jerry_value_is_typedarray(m_value); }
    bool IsArrayBuffer() const { return jerry_value_is_arraybuffer(m_value); }
    bool IsProxy() const { return false; }
    bool IsMap() const { return false; }
    bool IsMapIterator() const { return false; }
    bool IsSet() const { return false; }
    bool IsSetIterator() const { return false; }
    bool IsDate() const { return false; }
    bool IsRegExp() const { return false; }
    bool IsSharedArrayBuffer() const { return false; }
    bool IsAsyncFunction() const { return false; }
    bool IsNativeError() const { return false; }
    bool IsArrayBufferView() const { return false; }
    bool IsFloat64Array() const { return false; }
    bool IsUint8Array() const { return false; }
    bool IsDataView() const { return false; }

    double GetNumberValue(void) const { return jerry_get_number_value(m_value); }
    uint32_t GetUInt32Value(void) const { return (uint32_t)jerry_get_number_value(m_value); }
    int32_t GetInt32Value(void) const { return (int32_t)jerry_get_number_value(m_value); }
    int64_t GetInt64Value(void) const { return (int64_t)jerry_get_number_value(m_value); }
    bool GetBooleanValue(void) const { return jerry_get_boolean_value(m_value); }

    int GetStringLength(void) const { return jerry_get_string_size(m_value); }
    int GetStringUtf8Length(void) const { return jerry_get_utf8_string_size(m_value); }

    JerryValue* ToString(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_string(m_value));
    }

    JerryValue* ToInteger(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_number(m_value));
    }

    JerryValue* ToObject(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_object(m_value));
    }

    static JerryValue* NewExternal(void* ptr) {
        jerry_value_t object = jerry_create_object();
        jerry_set_object_native_pointer(object, ptr, &JerryV8ExternalTypeInfo);

        // TODO: move this to a better place so it'll be constructed once, maybe create a constructor and prototype?
        jerry_value_t conv_failer = BuildHelperMethod("", "this.toString = this.valueOf = function() { throw new TypeError('Invalid usage'); }");
        jerry_call_function(conv_failer, object, NULL, 0);
        jerry_release_value(conv_failer);

        return new JerryValue(object);
    }

    void* GetExternalData(void) const {
        void *native_p;
        bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8ExternalTypeInfo);
        if (!has_data) {
            return NULL;
        }

        return native_p;
    }

    bool IsExternal() const {
        void *native_p;
        bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8ExternalTypeInfo);
        return has_data;
    }

private:
    jerry_value_t m_value;
};

struct PropertyEntry {
    JerryValue* key;
    JerryValue* value;
    v8::PropertyAttribute attribute;
};

class JerryTemplate : public JerryHandle {
public:
    void Set(JerryValue* key, JerryValue* value,  v8::PropertyAttribute attribute) {
        m_properties.push_back(new PropertyEntry{key, value, attribute});
    }

    const std::vector<PropertyEntry*>& properties(void) const { return m_properties; }

protected:
    JerryTemplate(JerryHandle::Type type)
        : JerryHandle(type)
    {
    }

    std::vector<PropertyEntry*> m_properties;
};

class JerryObjectTemplate : public JerryTemplate {
public:
    JerryObjectTemplate()
        : JerryTemplate(ObjectTemplate)
    {
    }

};

class JerryFunctionTemplate : public JerryTemplate {
public:
    JerryFunctionTemplate()
        : JerryTemplate(FunctionTemplate)
        , m_prototype_template(NULL)
        , m_instance_template(NULL)
        , m_external(jerry_create_undefined())
    {
    }

    ~JerryFunctionTemplate(void) {
        if (m_prototype_template) {
            delete m_prototype_template;
        }

        if (m_instance_template) {
            delete m_instance_template;
        }

        jerry_release_value(m_external);
    }

    JerryObjectTemplate* PrototypeTemplate(void) {
        if (!m_prototype_template) {
            m_prototype_template = new JerryObjectTemplate();
        }

        return m_prototype_template;
    }

    JerryObjectTemplate* InstanceTemplate(void) {
        if (!m_instance_template) {
            m_instance_template = new JerryObjectTemplate();
        }

        return m_instance_template;
    }

    bool HasInstanceTemplate(void) const { return m_instance_template != NULL; }

    void SetCallback(v8::FunctionCallback callback) { m_callback = callback; }
    void SetExternalData(jerry_value_t value) { m_external = value; }

    v8::FunctionCallback callback(void) const { return m_callback; }
    jerry_value_t external(void) const { return m_external; }

private:
    JerryObjectTemplate* m_prototype_template;
    JerryObjectTemplate* m_instance_template;
    v8::FunctionCallback m_callback;
    jerry_value_t m_external;
};

struct JerryV8FunctionHandlerData {
    JerryFunctionTemplate* function_template;
    v8::FunctionCallback v8callback;
};

static void JerryV8FunctionHandlerDataFree(void* data) {
    delete reinterpret_cast<JerryV8FunctionHandlerData*>(data);
}

static jerry_object_native_info_t JerryV8FunctionHandlerTypeInfo = {
    .free_cb = JerryV8FunctionHandlerDataFree,
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

    void RemoveHandle(JerryHandle* jvalue) {
        // TODO: check if it really exists
        m_handles.erase(std::find(m_handles.begin(), m_handles.end(), jvalue));
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

        {/* new Map() method */
            const char* fn_args = "";
            const char* fn_body = "return new Map();";
            m_fn_map_new = new JerryValue(BuildHelperMethod(fn_args, fn_body));
        }
        {/* Map.Set helper method */
            const char* fn_args = "map, key, value";
            const char* fn_body = "return map.set(key, value);";

            m_fn_map_set = new JerryValue(BuildHelperMethod(fn_args, fn_body));
        }
    }

    ~JerryIsolate() {}

    const JerryValue& HelperMapNew(void) const { return *m_fn_map_new; }
    const JerryValue& HelperMapSet(void) const { return *m_fn_map_set; }

    void Enter(void) {
        JerryIsolate::s_currentIsolate = this;
    }

    void Exit(void) {
        JerryIsolate::s_currentIsolate = nullptr;
    }

    void Dispose(void) {
        delete m_fn_map_new;
        delete m_fn_map_set;
        jerry_cleanup();

        // Warning!... Do not use the JerryIsolate after this!
        // If you do: dragons will spawn from the depths of the earth and tear everything apart!
        // You have been warned!
        delete this;
    }

    void PushHandleScope(void* ptr) {
        m_handleScopes.push(new JerryHandleScope());
    }

    void PopHandleScope(void* ptr) {
        JerryHandleScope* handleScope = m_handleScopes.top();
        m_handleScopes.pop();

        delete handleScope;
    }

    JerryHandleScope* CurrentHandleScope(void) {
        return m_handleScopes.top();
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
    JerryValue* m_fn_map_new;
    JerryValue* m_fn_map_set;
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
    //for (JerryHandle* jhandle : m_handles) {
    for (std::vector<JerryHandle*>::reverse_iterator it = m_handles.rbegin();
        it != m_handles.rend();
        it++) {
        JerryHandle* jhandle = *it;
        switch (jhandle->type()) {
            case JerryHandle::Value: delete reinterpret_cast<JerryValue*>(jhandle); break;
            case JerryHandle::Context: delete reinterpret_cast<JerryContext*>(jhandle); break;
            case JerryHandle::FunctionTemplate: delete reinterpret_cast<JerryFunctionTemplate*>(jhandle); break;
            case JerryHandle::ObjectTemplate:
                // TODO: for now it is assumed that the functiontemplate will release the object template
                //delete reinterpret_cast<JerryObjectTemplate*>(jhandle);
                break;
            default: fprintf(stderr, "~JerryHandleScope::Unknown handle type (%d)\n", jhandle->type()); break;
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

/* base mutex*/
namespace base {
class Mutex {
public:
    Mutex() { }
    ~Mutex() { }
};

} // namespace base

const char* V8::GetVersion() {
    return "JerryScript v2.0";
}

/* V8 statics */
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

void V8::FromJustIsNothing() { }

void V8::ShutdownPlatform() { }

/* ArrayBuffer & Allocator */
Local<ArrayBuffer> ArrayBuffer::New(Isolate* isolate, void* data, size_t byte_length, ArrayBufferCreationMode mode) {
    jerry_value_t buffer;

    if (mode == ArrayBufferCreationMode::kInternalized) {
        buffer = jerry_create_arraybuffer(byte_length);
        jerry_arraybuffer_write(buffer, 0, (uint8_t*)data, byte_length);
    } else {
        buffer = jerry_create_arraybuffer_external(byte_length, (uint8_t*)data, nullptr);
    }

    RETURN_HANDLE(ArrayBuffer, isolate, new JerryValue(buffer));
}

ArrayBuffer::Contents ArrayBuffer::GetContents() {
    JerryValue* jbuffer = reinterpret_cast<JerryValue*> (this);
    jerry_value_t buffer = jbuffer->value();

    ArrayBuffer::Contents contents;
    contents.data_ = (void*) jerry_get_arraybuffer_pointer (buffer);
    contents.byte_length_ = (size_t) jerry_get_arraybuffer_byte_length (buffer);

    return contents;
}

size_t ArrayBuffer::ByteLength() const {
    const JerryValue* jbuffer = reinterpret_cast<const JerryValue*> (this);

    return (size_t) jerry_get_arraybuffer_byte_length (jbuffer->value());
}

void ArrayBuffer::Neuter() {
}

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

Isolate* Isolate::GetCurrent() {
    return JerryIsolate::toV8(JerryIsolate::GetCurrent());
}

void Isolate::LowMemoryNotification(void) { }

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

Local<Object> Context::Global() {
    RETURN_HANDLE(Object, GetIsolate(), new JerryValue(jerry_get_global_object()));
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

/* Value */
Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
    return Just((int32_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value());
}

int32_t Value::Int32Value() const {
    return (int32_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value();
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
    return Just((uint32_t)reinterpret_cast<const JerryValue*>(this)->GetUInt32Value());
}

uint32_t Value::Uint32Value() const {
    return reinterpret_cast<const JerryValue*> (this)->GetUInt32Value();
}

Maybe<bool> Value::BooleanValue(Local<Context> context) const {
    return Just((bool)reinterpret_cast<const JerryValue*>(this)->GetBooleanValue());
}

bool Value::BooleanValue() const {
    return (bool)reinterpret_cast<const JerryValue*>(this)->GetBooleanValue();
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
    return Just((double)reinterpret_cast<const JerryValue*>(this)->GetNumberValue());
}

double Value::NumberValue() const {
    return (double)reinterpret_cast<const JerryValue*>(this)->GetNumberValue();
}

int64_t Value::IntegerValue() const {
    return (int64_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value();
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
    return Just((int64_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value());
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToString();
    RETURN_HANDLE(String, context->GetIsolate(), result);
}

Local<String> Value::ToString(Isolate* isolate) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToString();
    RETURN_HANDLE(String, isolate, result);
}

Local<Integer> Value::ToInteger(Isolate* isolate) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToInteger();
    RETURN_HANDLE(Integer, isolate, result);
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToObject();
    RETURN_HANDLE(Object, Isolate::GetCurrent(), result);
}

Local<Object> Value::ToObject(Isolate* isolate) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToObject();
    RETURN_HANDLE(Object, isolate, result);
}

bool Value::IsBoolean() const {
    return reinterpret_cast<const JerryValue*> (this)->IsBoolean();
}

bool Value::IsFalse() const {
    return reinterpret_cast<const JerryValue*> (this)->IsFalse();
}

bool Value::IsTrue() const {
    return reinterpret_cast<const JerryValue*> (this)->IsTrue();
}

bool Value::IsPromise() const {
    return reinterpret_cast<const JerryValue*> (this)->IsPromise();
}

bool Value::IsArray() const {
    return reinterpret_cast<const JerryValue*> (this)->IsArray();
}

bool Value::IsObject() const {
    return reinterpret_cast<const JerryValue*> (this)->IsObject();
}

bool Value::IsNumber() const {
    return reinterpret_cast<const JerryValue*> (this)->IsNumber();
}

bool Value::IsUint32() const {
    return reinterpret_cast<const JerryValue*> (this)->IsUint32();
}

bool Value::IsInt32() const {
    return reinterpret_cast<const JerryValue*> (this)->IsInt32();
}

bool Value::IsFunction() const {
    return reinterpret_cast<const JerryValue*> (this)->IsFunction();
}

bool Value::IsSymbol() const {
    return reinterpret_cast<const JerryValue*> (this)->IsSymbol();
}

bool Value::IsTypedArray() const {
    return reinterpret_cast<const JerryValue*> (this)->IsTypedArray();
}

bool Value::IsArrayBuffer() const {
    return reinterpret_cast<const JerryValue*> (this)->IsArrayBuffer();
}

bool Value::IsProxy() const {
    return reinterpret_cast<const JerryValue*> (this)->IsProxy();
}

bool Value::IsMap() const {
    return reinterpret_cast<const JerryValue*> (this)->IsMap();
}

bool Value::IsMapIterator() const {
     return reinterpret_cast<const JerryValue*> (this)->IsMapIterator();
 }

bool Value::IsSet() const {
    return reinterpret_cast<const JerryValue*> (this)->IsSet();
}

bool Value::IsSetIterator() const {
    return reinterpret_cast<const JerryValue*> (this)->IsSetIterator();
}

bool Value::IsDate() const {
    return reinterpret_cast<const JerryValue*> (this)->IsDate();
}

bool Value::IsRegExp() const {
    return reinterpret_cast<const JerryValue*> (this)->IsRegExp();
}

bool Value::IsSharedArrayBuffer() const {
    return reinterpret_cast<const JerryValue*> (this)->IsSharedArrayBuffer();
}

bool Value::IsAsyncFunction() const {
    return reinterpret_cast<const JerryValue*> (this)->IsAsyncFunction();
}

bool Value::IsNativeError() const {
    return reinterpret_cast<const JerryValue*> (this)->IsNativeError();
}

bool Value::IsArrayBufferView() const {
    return reinterpret_cast<const JerryValue*> (this)->IsArrayBufferView();
}

bool Value::IsFloat64Array() const {
    return reinterpret_cast<const JerryValue*> (this)->IsFloat64Array();
}

bool Value::IsUint8Array() const {
    return reinterpret_cast<const JerryValue*> (this)->IsUint8Array();
}

bool Value::IsDataView() const {
    return reinterpret_cast<const JerryValue*> (this)->IsDataView();
}

bool Value::IsExternal() const {
    return reinterpret_cast<const JerryValue*> (this)->IsExternal();
}

/* Integer */
Local<Integer> Integer::New(Isolate* isolate, int32_t value) {
    jerry_value_t result = jerry_create_number(value);
    RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

Local<Integer> Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
    jerry_value_t result = jerry_create_number(value);
    RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

int64_t Integer::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetInt64Value();
}

/* Number */
Local<Number> Number::New(Isolate* isolate, double value) {
    jerry_value_t result = jerry_create_number(value);
    RETURN_HANDLE(Number, isolate, new JerryValue(result));
}

double Number::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetNumberValue();
}

/* UInt32 */
uint32_t Uint32::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetUInt32Value();
}

/* Int32 */
int32_t Int32::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetInt32Value();
}

/* Boolean */
bool Boolean::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetBooleanValue();
}

/* External */
/* External is a JS object */
Local<External> External::New(Isolate* isolate, void* value) {
    RETURN_HANDLE(External, isolate, JerryValue::NewExternal(value));
}

void* External::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetExternalData();
}

/* Object */
Local<Object> Object::New(Isolate* isolate) {
    jerry_value_t obj = jerry_create_object();

    RETURN_HANDLE(Object, isolate, new JerryValue(obj));
}

Maybe<bool> Object::Set(Local<Context> context, Local<Value> key, Local<Value> value) {
    return Just(reinterpret_cast<JerryValue*>(this)->SetProperty(
                    reinterpret_cast<JerryValue*>(*key),
                    reinterpret_cast<JerryValue*>(*value)));
}

bool Object::Set(Local<Value> key, Local<Value> value) {
    return reinterpret_cast<JerryValue*>(this)->SetProperty(
                reinterpret_cast<JerryValue*>(*key),
                reinterpret_cast<JerryValue*>(*value));
}

bool Object::Set(uint32_t index, Local<Value> value) {
    return reinterpret_cast<JerryValue*> (this)->SetPropertyIdx(index, reinterpret_cast<JerryValue*>(*value));
}

Isolate* Object::GetIsolate() {
    return Isolate::GetCurrent();
}

/* Array */
Local<Array> Array::New(Isolate* isolate, int length) {
    if (length < 0) {
        length = 0;
    }

    jerry_value_t array_value = jerry_create_array(length);
    RETURN_HANDLE(Array, isolate, new JerryValue(array_value));
}

uint32_t Array::Length() const {
    const JerryValue* array = reinterpret_cast<const JerryValue*>(this);

    return jerry_get_array_length(array->value());
}

/* Map */
Local<Map> Map::New(Isolate* isolate) {
    // TODO: add jerry api for map creation;
    jerry_value_t map_builder = JerryIsolate::fromV8(isolate)->HelperMapNew().value();
    jerry_value_t new_map = jerry_call_function(map_builder, jerry_create_undefined(), NULL, 0);

    RETURN_HANDLE(Map, isolate, new JerryValue(new_map));
}

MaybeLocal<Map> Map::Set(Local<Context> context, Local<Value> key, Local<Value> value) {
    JerryValue* jmap = reinterpret_cast<JerryValue*>(this);
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(*value);

    // TODO: ADD JS api / make sure the function is only created once.

    jerry_value_t map_set_helper = JerryIsolate::fromV8(context->GetIsolate())->HelperMapSet().value();
    jerry_value_t args[] = { jmap->value(), jkey->value(), jvalue->value() };
    jerry_value_t result = jerry_call_function(map_set_helper, jerry_create_undefined(), args, 3);
    jerry_release_value(result);

    return Local<Map>(this);
}

Local<Private> Private::New(Isolate* isolate, Local<String> name) {
    RETURN_HANDLE(Private ,isolate, reinterpret_cast<JerryValue*>(*name));
}

/* Symbol */
Local<Symbol> Symbol::New(Isolate* isolate, Local<String> name) {
    JerryValue* jname = reinterpret_cast<JerryValue*>(*name);

    jerry_value_t symbol_name = jerry_create_symbol (jname->value());
    jerry_release_value (symbol_name);

    RETURN_HANDLE(Symbol, isolate, new JerryValue(symbol_name));
}

/* Message */
Maybe<int> Message::GetStartColumn(v8::Local<v8::Context> context) const {
    return Just(0);
}

Maybe<int> Message::GetEndColumn(v8::Local<v8::Context> context) const {
    return Just(0);
}

int Message::GetLineNumber() const {
    return 0;
}

Local<Value> Message::GetScriptResourceName() const {
    return Local<Value>();
}

Local<String> Message::GetSourceLine() const {
    return Local<String>();
}

ScriptOrigin Message::GetScriptOrigin() const {
    return ScriptOrigin(Local<Value>());
}


/* String */
MaybeLocal<String> String::NewFromOneByte(
    Isolate* isolate, unsigned char const* data, v8::NewStringType type, int length /* = -1 */) {
    // TODO: what is the diff between the OneByte/TwoByte/Utf8 etc.?
    return String::NewFromUtf8(isolate, (const char*)data, (String::NewStringType)type, length);
}

Local<String> String::NewFromOneByte(
    Isolate* isolate, unsigned char const* data, String::NewStringType type /* = kNormalString */, int length /* = -1 */) {
    // TODO: what is the diff between the OneByte/TwoByte/Utf8 etc.?
    return String::NewFromUtf8(isolate, (const char*)data, (String::NewStringType)type, length);
}

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

int String::Length() const {
    return reinterpret_cast<const JerryValue*>(this)->GetStringLength();
}

int String::Utf8Length() const {
    return reinterpret_cast<const JerryValue*>(this)->GetStringUtf8Length();
}

String::Utf8Value::Utf8Value(Local<v8::Value> obj) : Utf8Value(Isolate::GetCurrent(), obj) { }

String::Utf8Value::Utf8Value(Isolate* isolate, Local<v8::Value> v8Value)
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
    delete [] str_;
}

String::Value::~Value() {
    delete [] str_;
}

Local<String> String::Concat(Local<String> left, Local<String> right) {
    JerryValue* lhs = reinterpret_cast<JerryValue*>(*left);
    JerryValue* rhs = reinterpret_cast<JerryValue*>(*right);

    jerry_size_t lsize = jerry_get_string_size (lhs->value());
    jerry_size_t rsize = jerry_get_string_size (rhs->value());

    char* buffer = new char[lsize + rsize];

    jerry_string_to_char_buffer (lhs->value(), (jerry_char_t*) buffer, lsize);
    jerry_string_to_char_buffer (lhs->value(), (jerry_char_t*) buffer + lsize, rsize);

    jerry_value_t value = jerry_create_string_sz ((jerry_char_t*)buffer, lsize + rsize);

    RETURN_HANDLE(String, Isolate::GetCurrent(), new JerryValue(value));
}

/* Script */
MaybeLocal<Script> Script::Compile(Local<Context> context, Local<String> source, ScriptOrigin* origin /* = nullptr */) {
    jerry_char_t* sourceString = new jerry_char_t[source->Utf8Length() + 1];
    source->WriteUtf8((char*)sourceString, source->Utf8Length(), 0, 0);

    jerry_value_t scriptFunction = jerry_parse(NULL, 0, sourceString, source->Utf8Length(), JERRY_PARSE_NO_OPTS);

    delete [] sourceString;

    RETURN_HANDLE(Script, context->GetIsolate(), new JerryValue(scriptFunction));
}

MaybeLocal<Value> Script::Run(v8::Local<v8::Context> context) {
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(this);

    jerry_value_t result = jerry_run(jvalue->value());

    RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(result));
}

Local<Value> Script::Run() {
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(this);

    jerry_value_t result = jerry_run(jvalue->value());
    // TODO: report error for try-catch.

    RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(result));
}

/* Function */
Local<Object> Function::NewInstance(int argc, Local<Value> argv[]) const {
    const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

    std::vector<jerry_value_t> arguments;
    arguments.resize(argc);
    for (int idx = 0; idx < argc; idx++) {
        JerryValue* arg = reinterpret_cast<JerryValue*>(*argv[idx]);
        arguments[idx] = arg->value();
    }

    JerryValue* object = new JerryValue(jerry_construct_object(jvalue->value(), &arguments[0], (jerry_size_t)argc));
    RETURN_HANDLE(Object, Isolate::GetCurrent(), object);
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context, int argc, Local<Value> argv[]) const {
    return NewInstance(argc, argv);
}

/* Function Template */
template<typename T>
class JerryFunctionCallbackInfo : public FunctionCallbackInfo<T> {
/* Form parent:
  static const int kHolderIndex = 0;
  static const int kIsolateIndex = 1;
  static const int kReturnValueDefaultValueIndex = 2;
  static const int kReturnValueIndex = 3;
  static const int kDataIndex = 4;
  static const int kCalleeIndex = 5;
  static const int kContextSaveIndex = 6;
static const int kNewTargetIndex = 7;
*/
public:
    static const int kImplicitArgsSize = FunctionCallbackInfo<T>::kNewTargetIndex + 1;

    JerryFunctionCallbackInfo(
        const jerry_value_t function_obj,
        const jerry_value_t this_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt,
        const JerryV8FunctionHandlerData* handlerData)
        // Please note there is a "Hack"/"Fix" in the v8.h file where the FunctionCallbackInfo's "values_" member is accessed!
        : FunctionCallbackInfo<T>(reinterpret_cast<internal::Object**>(BuildImplicitArgs(function_obj, this_val, handlerData)),
                                reinterpret_cast<internal::Object**>(BuildArgs(this_val, args_p, args_cnt) + args_cnt - 1),
                                args_cnt)
        // Beware of magic: !!!
        , m_values(reinterpret_cast<JerryHandle**>(this->values_) - args_cnt + 1)
    {}

    ~JerryFunctionCallbackInfo() {
        const int args_count = this->Length() + 1; /* +1 is the js 'this' */
        for (int idx = 0; idx < args_count; idx++) {
            delete m_values[idx];
        }
        delete [] m_values;

        JerryHandle **implicit_args = reinterpret_cast<JerryHandle**>(this->implicit_args_);
        delete [] implicit_args;
    }

private:
    static JerryHandle** BuildArgs(
        const jerry_value_t this_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt) {

        JerryHandle **values = new JerryHandle*[args_cnt + 1 /* this */];
        /* this should be at 'values - 1' */

        /* args_p[0]  is at 'values - 0' */
        /* args_p[1]  is at 'values - 1' */
        /* args_p[2]  is at 'values - 2' */
        for (jerry_length_t idx = 0; idx < args_cnt; idx++) {
            values[args_cnt - idx - 1] = new JerryValue(args_p[idx]);
        }
        values[args_cnt] = new JerryValue(this_val);

        return values;
    }

    static JerryHandle** BuildImplicitArgs(const jerry_value_t function_obj,
                                           const jerry_value_t this_val,
                                           const JerryV8FunctionHandlerData* handlerData) {

        JerryHandle **values = new JerryHandle*[kImplicitArgsSize];
        values[FunctionCallbackInfo<T>::kHolderIndex] = new JerryValue(jerry_acquire_value(this_val));
        // TODO: correctly fill the arguments:
        values[FunctionCallbackInfo<T>::kIsolateIndex] = reinterpret_cast<JerryHandle*>(Isolate::GetCurrent()); /* isolate; */
        values[FunctionCallbackInfo<T>::kReturnValueIndex] = new JerryValue(jerry_create_undefined()); /*construct_call ? this : nullptr;*/
        values[FunctionCallbackInfo<T>::kDataIndex] = new JerryValue(jerry_acquire_value(handlerData->function_template->external())); /* data; */
        values[FunctionCallbackInfo<T>::kCalleeIndex] = NULL; /* callee; */
        values[FunctionCallbackInfo<T>::kNewTargetIndex] = NULL; /* new_target; */

        // HandleScope will do the cleanup for us at a later stage
        JerryIsolate::GetCurrent()->AddToHandleScope(values[FunctionCallbackInfo<T>::kHolderIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[FunctionCallbackInfo<T>::kReturnValueIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[FunctionCallbackInfo<T>::kDataIndex]);

        return values;
    }

    JerryHandle** m_values;
};

static jerry_value_t JerryV8FunctionHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt) {

    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(function_obj, &native_p, &JerryV8FunctionHandlerTypeInfo);

    if (!has_data) {
        fprintf(stderr, "ERRR.... should not be here!(%s:%d)\n", __FILE__, __LINE__);
        abort();
        return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t*)"BAD!!");
    }
    JerryV8FunctionHandlerData* data = reinterpret_cast<JerryV8FunctionHandlerData*>(native_p);

    // Make sure that Localy allocated vars will be freed upon exit.
    v8::HandleScope handle_scope(Isolate::GetCurrent());

    if (jerry_value_is_object(this_val)) {
        JerryValue* object = new JerryValue(jerry_acquire_value(this_val));

        if (data->function_template->HasInstanceTemplate()) {
            JerryObjectTemplate* tmplt = data->function_template->InstanceTemplate();

            // TODO: move to a general place (see FunctionTemplate::GetFunction)
            for(PropertyEntry* prop : tmplt->properties()) {
                // TODO: do not ignore the prop->attributes
                object->SetProperty(prop->key, prop->value);
            }
        }
        delete object;
    }


    JerryFunctionCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, data);

    data->v8callback(info);

    v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

    // Again: dragons!
    JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

    jerry_value_t jret = jerry_acquire_value(retVal->value());

    // No need to delete the JerryValue here, the HandleScope will take (should) care of it!
    //delete retVal;

    return jret;
}

void FunctionTemplate::SetCallHandler(FunctionCallback callback,
                                      v8::Local<Value> data) {
    JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
    func->SetCallback(callback);
    if (!data.IsEmpty()) {
        JerryValue* value = reinterpret_cast<JerryValue*>(*data);
        jerry_value_t jvalue = jerry_acquire_value(value->value());

        func->SetExternalData(jvalue);
    }
}

Local<FunctionTemplate> FunctionTemplate::New(
    Isolate* isolate, FunctionCallback callback /*= 0*/, Local<Value> data /*= Local<Value>()*/,
    Local<Signature> signature /* = Local<Signature>() */, int length /* = 0 */, ConstructorBehavior behavior /* = ConstructorBehavior::kAllow */) {
    // TODO: handle the other args
    JerryFunctionTemplate* func = new JerryFunctionTemplate();
    reinterpret_cast<FunctionTemplate*>(func)->SetCallHandler(callback, data);

    RETURN_HANDLE(FunctionTemplate, isolate, func);
}

Local<Function> FunctionTemplate::GetFunction() {
    JerryFunctionTemplate* tmplt = reinterpret_cast<JerryFunctionTemplate*>(this);

    jerry_value_t jfunction = jerry_create_external_function(JerryV8FunctionHandler);

    JerryV8FunctionHandlerData* data = new JerryV8FunctionHandlerData();
    data->function_template = tmplt; // Required so we can do instance checks.
    data->v8callback = tmplt->callback();

    jerry_set_object_native_pointer(jfunction, data, &JerryV8FunctionHandlerTypeInfo);

    JerryValue* func = new JerryValue(jfunction);

    // TODO: move to a general place (see Function::NewInstance)
    for(PropertyEntry* prop : tmplt->properties()) {
        // TODO: do not ignore the prop->attributes
        func->SetProperty(prop->key, prop->value);
    }

    RETURN_HANDLE(Function, Isolate::GetCurrent(), func);
}

void FunctionTemplate::SetClassName(Local<String> name) {
    // TODO: This should be used as the constructor's name. Skip this for now.
}

bool FunctionTemplate::HasInstance(Local<Value> object) {
    JerryValue* value = reinterpret_cast<JerryValue*>(*object);

    if (!value->IsFunction()) {
        return false;
    }

    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(value->value(), &native_p, &JerryV8FunctionHandlerTypeInfo);

    if (!has_data) {
        return false;
    }

    JerryV8FunctionHandlerData* data = reinterpret_cast<JerryV8FunctionHandlerData*>(native_p);

    // TODO: the prototype chain should be traversed

    // TODO: do a better check not just a simple address check
    return data->function_template == reinterpret_cast<JerryFunctionTemplate*>(this);
}

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
    JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
    JerryObjectTemplate* obj_template = func->PrototypeTemplate();
    RETURN_HANDLE(ObjectTemplate, Isolate::GetCurrent(), obj_template);
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
    JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
    JerryObjectTemplate* obj_template = func->InstanceTemplate();
    RETURN_HANDLE(ObjectTemplate, Isolate::GetCurrent(), obj_template);
}

/*  ObjectTemplate */
void Template::Set(v8::Local<v8::Name> name, v8::Local<v8::Data> data, v8::PropertyAttribute attributes) {
    JerryTemplate* templt = reinterpret_cast<JerryTemplate*>(this);

    JerryValue* key = reinterpret_cast<JerryValue*>(*name);
    // TODO: maybe this should not be a JerryValue?
    JerryValue* value = reinterpret_cast<JerryValue*>(*data);

    templt->Set(key, value, attributes);
}

/* Stackframe */
int StackFrame::GetColumn() const {
    return 5;
}

int StackFrame::GetScriptId() const {
    return 0;
}

bool StackFrame::IsEval() const {
    return true;
}

int StackTrace::GetFrameCount() const {
    return 0;
}

Local<String> StackFrame::GetFunctionName() const {
    return Local<String>();
}

int StackFrame::GetLineNumber() const {
    return 0;
}

Local<String> StackFrame::GetScriptName() const {
    return Local<String>();
}

/* Dummy tracing */
namespace platform {
namespace tracing {

TraceBufferChunk::TraceBufferChunk(uint32_t seq) { }

TraceObject* TraceBufferChunk::AddTraceEvent(size_t* event_index) {
    return NULL;
}

void TraceBufferChunk::Reset(uint32_t new_seq) { }


void TraceConfig::AddIncludedCategory(char const* included_category) { }

TraceObject::~TraceObject() { }

TraceWriter* TraceWriter::CreateJSONTraceWriter(std::ostream& stream) {
    return NULL;
}

TracingController::TracingController() { }
TracingController::~TracingController() { }
void TracingController::Initialize(TraceBuffer* trace_buffer) { }
void TracingController::StartTracing(TraceConfig* trace_config) { }
void TracingController::StopTracing() { }
const uint8_t* TracingController::GetCategoryGroupEnabled(const char* category_group) {
    return NULL;
}

uint64_t TracingController::AddTraceEvent(
        char phase, const uint8_t* category_enabled_flag, const char* name,
        const char* scope, uint64_t id, uint64_t bind_id, int32_t num_args,
        const char** arg_names, const uint8_t* arg_types,
        const uint64_t* arg_values,
        std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
        unsigned int flags) {
    return 0;
}

void TracingController::UpdateTraceEventDuration(const uint8_t* category_enabled_flag, const char* name, uint64_t handle) { }
void TracingController::AddTraceStateObserver(v8::TracingController::TraceStateObserver* observer) { }
void TracingController::RemoveTraceStateObserver(v8::TracingController::TraceStateObserver* observer) { }

} // namespace tracing
} // namespace platform
/* Dummy tracing END*/

} // namespace v8
