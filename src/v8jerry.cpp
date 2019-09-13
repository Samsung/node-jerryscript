#include <v8.h>
#include <v8-debug.h>
#include <v8-profiler.h>
#include <libplatform/libplatform.h>

#include <cassert>
#include <cstring>
#include <deque>
#include <stack>
#include <vector>
#include <algorithm>

#include "jerryscript.h"
#include "jerryscript-port-default.h"

/* Jerry <-> V8 binding classes */
#include "v8jerry_callback.hpp"
#include "v8jerry_context.hpp"
#include "v8jerry_handlescope.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_platform.hpp"
#include "v8jerry_templates.hpp"
#include "v8jerry_value.hpp"

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

size_t TypedArray::Length() {
    return (size_t) jerry_get_typedarray_length (reinterpret_cast<JerryValue*> (this)->value());
}

size_t ArrayBufferView::ByteLength() {
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;
    jerry_length_t byteLength = 0;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), NULL, &byteLength);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), NULL, &byteLength);
    } else {
        printf("Unknown object...\n");
    }

    jerry_release_value(buffer);

    return byteLength;
}

size_t ArrayBufferView::ByteOffset() {
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;
    jerry_length_t byteOffset = 0;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), &byteOffset, NULL);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), &byteOffset, NULL);
    } else {
        printf("Unknown object...\n");
    }

    jerry_release_value(buffer);
    return byteOffset;
}

Local<ArrayBuffer> ArrayBufferView::Buffer() {
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), NULL, NULL);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), NULL, NULL);
    } else {
        printf("Unknown object...\n");
    }

    RETURN_HANDLE(ArrayBuffer, Isolate::GetCurrent(), new JerryValue(buffer));
}

#define ArrayBufferView(view_class, view_type) \
    Local<view_class> view_class::New(Local<ArrayBuffer> array_buffer, size_t byte_offset, size_t length) { \
        JerryValue* jarraybuffer = reinterpret_cast<JerryValue*> (*array_buffer); \
        jerry_value_t arrayview = jerry_create_typedarray_for_arraybuffer_sz (view_type, jarraybuffer->value(), byte_offset, length); \
        if (jerry_value_is_error(arrayview)) { \
            printf("Error at Typedarray creation...\n"); \
        } \
        RETURN_HANDLE(view_class, Isolate::GetCurrent(), new JerryValue(arrayview)); \
    }

ArrayBufferView(Uint8Array, JERRY_TYPEDARRAY_UINT8);
ArrayBufferView(Uint32Array, JERRY_TYPEDARRAY_UINT32);
ArrayBufferView(Float64Array, JERRY_TYPEDARRAY_FLOAT64);

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

void Isolate::SetFatalErrorHandler(FatalErrorCallback that) {
    JerryIsolate::fromV8(this)->SetFatalErrorHandler(that);
}

void Isolate::GetHeapStatistics(HeapStatistics*) { }

HeapProfiler* Isolate::GetHeapProfiler() {
    return NULL;
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
    JerryIsolate::fromV8(isolate_)->PushHandleScope(JerryHandleScopeType::Normal, this);
}

HandleScope::~HandleScope(void) {
    JerryIsolate::fromV8(isolate_)->PopHandleScope(this);
}

internal::Object** HandleScope::CreateHandle(internal::Isolate* isolate, internal::Object* value) {
    JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(value);
    switch (jhandle->type()) {
        case JerryHandle::FunctionTemplate:
        case JerryHandle::ObjectTemplate:
            reinterpret_cast<JerryIsolate*>(isolate)->AddTemplate(reinterpret_cast<JerryTemplate*>(jhandle));
            break;
        default:
            reinterpret_cast<JerryIsolate*>(isolate)->AddToHandleScope(jhandle);
            break;
    }
    return reinterpret_cast<internal::Object**>(jhandle);
}

/* EscapableHandleScope */
EscapableHandleScope::EscapableHandleScope(Isolate* isolate)
    : HandleScope(isolate)
{
}

internal::Object** EscapableHandleScope::Escape(internal::Object** value) {
    JerryIsolate::fromV8(GetIsolate())->EscapeHandle(reinterpret_cast<JerryValue*>(value));
    return value;
}

/* SealHandleScope */
SealHandleScope::SealHandleScope(Isolate* isolate)
    : isolate_(reinterpret_cast<v8::internal::Isolate* const>(isolate))
{
    JerryIsolate::fromV8(isolate_)->PushHandleScope(JerryHandleScopeType::Sealed, this);
}

SealHandleScope::~SealHandleScope() {
    JerryIsolate::fromV8(isolate_)->PopHandleScope(this);
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
    const JerryValue* jval = reinterpret_cast<const JerryValue*> (this);

    jerry_value_t arg = jval->value();
    jerry_value_t result =
        JerryIsolate::fromV8(v8::Isolate::GetCurrent())->HelperIsMap().Call(jerry_create_undefined(), &arg, 1);

    bool isMap = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isMap;
}

bool Value::IsMapIterator() const {
     return reinterpret_cast<const JerryValue*> (this)->IsMapIterator();
}

bool Value::IsSet() const {
    const JerryValue* jval = reinterpret_cast<const JerryValue*> (this);

    jerry_value_t arg = jval->value();
    jerry_value_t result =
        JerryIsolate::fromV8(v8::Isolate::GetCurrent())->HelperIsSet().Call(jerry_create_undefined(), &arg, 1);

    bool isSet = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isSet;
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

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
    const JerryValue* jValue = reinterpret_cast<const JerryValue*> (this);

    if (jValue->IsSymbol()) {
        jerry_value_t string_value = jerry_create_string ((const jerry_char_t *) "Symbol()");
        RETURN_HANDLE(String, context->GetIsolate(), new JerryValue(string_value));
    } else if (jValue->IsProxy()) {
        jerry_value_t string_value = jerry_create_string ((const jerry_char_t *) "[object Object]");
        RETURN_HANDLE(String, context->GetIsolate(), new JerryValue(string_value));
    }

    RETURN_HANDLE(String, context->GetIsolate(), jValue->ToString());
}

bool Value::Equals(Local<Value> value) const {
    const JerryValue* lhs = reinterpret_cast<const JerryValue*> (this);
    JerryValue* rhs = reinterpret_cast<JerryValue*> (*value);

    jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_EQUAL, lhs->value(), rhs->value());
    bool isEqual = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isEqual;
}

bool Value::StrictEquals(Local<Value> value) const {
    const JerryValue* lhs = reinterpret_cast<const JerryValue*> (this);
    JerryValue* rhs = reinterpret_cast<JerryValue*> (*value);

    jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_STRICT_EQUAL, lhs->value(), rhs->value());
    bool isEqual = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isEqual;
}

/* Integer */
Local<Integer> Integer::New(Isolate* isolate, int32_t value) {
    jerry_value_t result = jerry_create_number(value);
    RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

Local<Integer> Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
    jerry_value_t result = jerry_create_number((int32_t) value);
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
    RETURN_HANDLE(Object, isolate, JerryValue::NewObject());
}

Local<Context> Object::CreationContext(void) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

    JerryContext* jctx = jobj->GetObjectCreationContext();
    // Copy the context
    RETURN_HANDLE(Context, GetIsolate(), new JerryContext(*jctx));
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

Maybe<bool> Object::Set(Local<Context> context, uint32_t index, Local<Value> value) {
    return Just(reinterpret_cast<JerryValue*> (this)->SetPropertyIdx(index, reinterpret_cast<JerryValue*>(*value)));
}

bool Object::Set(uint32_t index, Local<Value> value) {
    return reinterpret_cast<JerryValue*> (this)->SetPropertyIdx(index, reinterpret_cast<JerryValue*>(*value));
}

Local<Value> Object::Get(uint32_t index) {
    RETURN_HANDLE(Value, Isolate::GetCurrent(), reinterpret_cast<JerryValue*> (this)->GetPropertyIdx(index));
}

MaybeLocal<Value> Object::Get(Local<Context> context, uint32_t index) {
    RETURN_HANDLE(Value, context->GetIsolate(), reinterpret_cast<JerryValue*> (this)->GetPropertyIdx(index));
}

Local<Value> Object::Get(Local<Value> key) {
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

    RETURN_HANDLE(Value, Isolate::GetCurrent(), reinterpret_cast<JerryValue*> (this)->GetProperty(jkey));
}

MaybeLocal<Value> Object::Get(Local<Context> context, Local<Value> key) {
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

    RETURN_HANDLE(Value, context->GetIsolate(), reinterpret_cast<JerryValue*> (this)->GetProperty(jkey));
}

Maybe<bool> Object::Delete(Local<Context> context, Local<Value> key) {
    JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
    JerryValue* jkey = reinterpret_cast<JerryValue*> (*key);

    return Just(jerry_delete_property (jobj->value(), jkey->value()));
}

Maybe<bool> Object::Has(Local<Context> context, Local<Value> key) {
    JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
    JerryValue* jkey = reinterpret_cast<JerryValue*> (*key);

    jerry_value_t has_prop_js = jerry_has_property (jobj->value(), jkey->value());
    bool has_prop = jerry_get_boolean_value (has_prop_js);
    jerry_release_value (has_prop_js);

    return Just(has_prop);
}

MaybeLocal<Value> Object::GetPrivate(Local<Context> context, Local<Private> key) {
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

    RETURN_HANDLE(Value, Isolate::GetCurrent(), reinterpret_cast<JerryValue*> (this)->GetProperty(jkey));
}

Maybe<bool> Object::HasPrivate(Local<Context> context, Local<Private> key) {
    JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
    JerryValue* jkey = reinterpret_cast<JerryValue*> (*key);

    jerry_value_t has_prop_js = jerry_has_property (jobj->value(), jkey->value());
    bool has_prop = jerry_get_boolean_value (has_prop_js);
    jerry_release_value (has_prop_js);

    return Just(has_prop);
}

Maybe<bool> Object::SetPrivate(Local<Context> context, Local<Private> key, Local<Value> value) {
    return Just(reinterpret_cast<JerryValue*>(this)->SetProperty(
                    reinterpret_cast<JerryValue*>(*key),
                    reinterpret_cast<JerryValue*>(*value)));
}

Maybe<bool> Object::DefineOwnProperty(Local<Context> context, Local<Name> key, Local<Value> value, PropertyAttribute attributes) {
    JerryValue* obj = reinterpret_cast<JerryValue*> (this);
    JerryValue* prop_name = reinterpret_cast<JerryValue*> (*key);
    JerryValue* prop_value = reinterpret_cast<JerryValue*> (*value);

    jerry_property_descriptor_t prop_desc = {
        .is_value_defined = true,
        .is_get_defined = false,
        .is_set_defined = false,
        .is_writable_defined = true,
        .is_writable = attributes & !PropertyAttribute::ReadOnly,
        .is_enumerable_defined = true,
        .is_enumerable = attributes & !PropertyAttribute::DontEnum,
        .is_configurable_defined = true,
        .is_configurable = attributes & !PropertyAttribute::DontDelete,
        .value = prop_value->value(),
        .getter = jerry_create_undefined(),
        .setter = jerry_create_undefined()
    };

    jerry_value_t result = jerry_define_own_property (obj->value(), prop_name->value(), &prop_desc);
    bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return Just(isOk);
}

Maybe<bool> Object::SetPrototype(Local<Context> context, Local<v8::Value> prototype) {
    return Just(SetPrototype(prototype));
}

bool Object::SetPrototype(Local<Value> prototype) {
    JerryValue* obj = reinterpret_cast<JerryValue*> (this);
    JerryValue* proto = reinterpret_cast<JerryValue*> (*prototype);

    jerry_value_t result = jerry_set_prototype (obj->value(), proto->value());
    bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isOk;
}

Isolate* Object::GetIsolate() {
    return Isolate::GetCurrent();
}

Maybe<bool> Object::SetAccessor(Local<Context> context,
                                Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter, /* = 0 */
                                MaybeLocal<Value> data, /* = MaybeLocal<Value>()*/
                                AccessControl settings, /* = DEFAULT */
                                PropertyAttribute attribute /* = None */) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

    JerryValue* jdata = NULL;
    if (!data.IsEmpty()) {
        Local<Value> dataValue;
        bool isOk = data.ToLocal(&dataValue);
        (void)isOk; // the "emptyness" is alread checked this should be always true,
        // TODO: maybe assert on "isOK"?

        jdata = reinterpret_cast<JerryValue*>(*dataValue)->Copy();
    }

    AccessorEntry entry(
        reinterpret_cast<JerryValue*>(*name)->Copy(),
        getter,
        setter,
        jdata,
        settings,
        attribute
    );

    bool configured = JerryObjectTemplate::SetAccessor(jobj->value(), entry);
    return Just(configured);
}

void Object::SetInternalField(int idx, Local<Value> value) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    // Indexing starts from 0!

    jobj->SetInternalField(idx, reinterpret_cast<JerryValue*>(*value));
}

void Object::SetAlignedPointerInInternalField(int idx, void* value) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    // Indexing starts from 0!

    jobj->SetInternalField(idx, value);
}

int Object::InternalFieldCount(void) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    return jobj->InternalFieldCount();
}

void* Object::SlowGetAlignedPointerFromInternalField(int idx) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    return jobj->GetInternalField<void*>(idx);
}

Local<Value> Object::SlowGetInternalField(int idx) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    RETURN_HANDLE(Value, Isolate::GetCurrent(), jobj->GetInternalField<JerryValue*>(idx));
}

Local<Object> Object::Clone(void) {
    // shallow copy!
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

    jerry_value_t arg = jobj->value();
    jerry_value_t result = JerryIsolate::fromV8(GetIsolate())->HelperObjectAssign().Call(jerry_create_undefined(), &arg, 1);

    RETURN_HANDLE(Object, GetIsolate(), new JerryValue(result));
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
    jerry_value_t new_map = JerryIsolate::fromV8(isolate)->HelperMapNew().Call(jerry_create_undefined(), NULL, 0);

    RETURN_HANDLE(Map, isolate, new JerryValue(new_map));
}

MaybeLocal<Map> Map::Set(Local<Context> context, Local<Value> key, Local<Value> value) {
    JerryValue* jmap = reinterpret_cast<JerryValue*>(this);
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(*value);

    // TODO: ADD JS api / make sure the function is only created once.

    jerry_value_t args[] = { jmap->value(), jkey->value(), jvalue->value() };
    jerry_value_t result = JerryIsolate::fromV8(context->GetIsolate())->HelperMapSet().Call(jerry_create_undefined(), args, 3);
    jerry_release_value(result);

    return Local<Map>(this);
}

Local<Private> Private::New(Isolate* isolate, Local<String> name) {
    JerryValue* jname = reinterpret_cast<JerryValue*>(*name);

    jerry_size_t size = jerry_get_string_size (jname->value());
    std::vector<char> buffer(size);

    jerry_length_t copied =
        jerry_string_to_char_buffer (jname->value(), reinterpret_cast<jerry_char_t*>(&buffer[0]), size);

    std::string private_name("$$private_");
    private_name.append(buffer.data(), copied);

    jerry_value_t private_key = jerry_create_string_sz_from_utf8((const jerry_char_t*)private_name.c_str(), private_name.size());

    RETURN_HANDLE(Private, isolate, new JerryValue(private_key));
}

/* Symbol */
Local<Symbol> Symbol::New(Isolate* isolate, Local<String> name) {
    JerryValue* jname = reinterpret_cast<JerryValue*>(*name);

    jerry_value_t symbol_name = jerry_create_symbol (jname->value());
    RETURN_HANDLE(Symbol, isolate, new JerryValue(symbol_name));
}

/* Proxy */
Local<Object> Proxy::GetTarget() {
    // Proxy is not supported by JerryScript.
    return Local<Object>();
}

Local<Value> Proxy::GetHandler() {
    // Proxy is not supported by JerryScript.
    return Local<Value>();
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

    if (jvalue == NULL || jvalue->value() == 0) {
        return;
    }

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

    std::vector<char> buffer;
    buffer.resize(lsize + rsize);

    jerry_string_to_char_buffer (lhs->value(), reinterpret_cast<jerry_char_t*>(&buffer[0]), lsize);
    jerry_string_to_char_buffer (rhs->value(), reinterpret_cast<jerry_char_t*>(&buffer[0]) + lsize, rsize);

    jerry_value_t value = jerry_create_string_sz (reinterpret_cast<const jerry_char_t*>(&buffer[0]), lsize + rsize);

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
MaybeLocal<Function> Function::New(Local<Context> context,
                                   FunctionCallback callback,
                                   Local<Value> data, /* = Local<Value>() */
                                   int length, /* = 0 */
                                   ConstructorBehavior behavior /* = ConstructorBehavior::kAllow */) {
    // TODO: maybe don't use function template?
    Local<FunctionTemplate> tmplt = FunctionTemplate::New(context->GetIsolate(), callback, data, Local<Signature>(), length, behavior);
    return tmplt->GetFunction();
}

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

Local<Value> Function::Call(Local<Value> recv, int argc, Local<Value> argv[]) {
    const JerryValue* jfunc = reinterpret_cast<const JerryValue*>(this);
    const JerryValue* jthis = reinterpret_cast<const JerryValue*>(*recv);

    std::vector<jerry_value_t> arguments;
    arguments.resize(argc);
    for (int idx = 0; idx < argc; idx++) {
        arguments[idx] = reinterpret_cast<JerryValue*>(*argv[idx])->value();
    }

    jerry_value_t result = jerry_call_function(jfunc->value(), jthis->value(), &arguments[0], argc);
    RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(result));
}

MaybeLocal<Value> Function::Call(Local<Context> context, Local<Value> recv, int argc, Local<Value> argv[]) {
    return Call(recv, argc, argv);
}

void Function::SetName(Local<String> name) {
    // TODO: how to set the "name" of a function (in JS the function.name is readonly)
}

Local<Value> Function::GetDebugName() const {
    // TODO: only used by node_perf.cc
    return Local<Value>();
}

Local<Value> Function::GetBoundFunction() const {
    // TODO: only used by node_perf.cc
    return Local<Value>();
}


/* Function Template */
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

Local<FunctionTemplate> FunctionTemplate::New(Isolate* isolate,
                                              FunctionCallback callback /*= 0*/,
                                              Local<Value> data /*= Local<Value>()*/,
                                              Local<Signature> signature /* = Local<Signature>() */,
                                              int length /* = 0 */,
                                              ConstructorBehavior behavior /* = ConstructorBehavior::kAllow */) {
    // TODO: handle the other args
    JerryFunctionTemplate* func = new JerryFunctionTemplate();
    reinterpret_cast<FunctionTemplate*>(func)->SetCallHandler(callback, data);

    RETURN_HANDLE(FunctionTemplate, isolate, func);
}

Local<Function> FunctionTemplate::GetFunction() {
    JerryFunctionTemplate* tmplt = reinterpret_cast<JerryFunctionTemplate*>(this);
    JerryValue *func = tmplt->GetFunction()->Copy();

    RETURN_HANDLE(Function, Isolate::GetCurrent(), func);
}

void FunctionTemplate::SetClassName(Local<String> name) {
    // TODO: This should be used as the constructor's name. Skip this for now.
}

bool FunctionTemplate::HasInstance(Local<Value> object) {
    JerryValue* value = reinterpret_cast<JerryValue*>(*object);

    if (!value->IsObject() || value->IsFunction()) {
        return false;
    }

    JerryV8FunctionHandlerData* data = JerryGetFunctionHandlerData(value->value());
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

/*  Template */
void Template::Set(v8::Local<v8::Name> name, v8::Local<v8::Data> data, v8::PropertyAttribute attributes) {
    JerryTemplate* templt = reinterpret_cast<JerryTemplate*>(this);

    // Here we copy the values as the template's internl elements will do a release on them
    JerryValue* key = reinterpret_cast<JerryValue*>(*name)->Copy();
    // TODO: maybe this should not be a JerryValue?
    JerryValue* value = reinterpret_cast<JerryValue*>(*data)->Copy();

    templt->Set(key, value, attributes);
}

void Template::SetAccessorProperty(
    Local<Name> name,
    Local<FunctionTemplate> getter /* = Local<FunctionTemplate>()*/,
    Local<FunctionTemplate> setter /* = Local<FunctionTemplate>()*/,
    PropertyAttribute attribute /* = None */,
    AccessControl settings /*= DEFAULT */) {

    JerryValue* key = reinterpret_cast<JerryValue*>(*name)->Copy();
    // TODO: maybe this should not be a JerryValue?

    JerryValue* jgetter = NULL;
    JerryValue* jsetter = NULL;
    if (!getter.IsEmpty()) {

        jgetter = reinterpret_cast<JerryFunctionTemplate*>(*getter)->GetFunction()->Copy();
    }

    if (!setter.IsEmpty()) {
        jgetter = reinterpret_cast<JerryFunctionTemplate*>(*setter)->GetFunction()->Copy();
    }
    // TODO: handle attributes and settings

    JerryTemplate* templt = reinterpret_cast<JerryTemplate*>(this);
    templt->SetAccessorProperty(key, jgetter, jsetter, attribute);
}

/* ObjectTemplate */
Local<ObjectTemplate> ObjectTemplate::New(Isolate* isolate, Local<FunctionTemplate> constructor /* = Local<FunctionTemplate>() */) {
    JerryObjectTemplate* obj_template = new JerryObjectTemplate();

    if (!constructor.IsEmpty()) {
        // TODO: check if this is correct
        JerryFunctionTemplate* function_template = reinterpret_cast<JerryFunctionTemplate*>(*constructor);
        obj_template->SetConstructor(function_template);
    }

    RETURN_HANDLE(ObjectTemplate, isolate, obj_template);
}

MaybeLocal<Object> ObjectTemplate::NewInstance(Local<Context> context) {
    JerryObjectTemplate* object_template = reinterpret_cast<JerryObjectTemplate*>(this);

    // TODO: the function template's method should be set as the object's constructor
    JerryValue* new_instance = JerryValue::NewObject();
    object_template->InstallProperties(new_instance->value());

    RETURN_HANDLE(Object, context->GetIsolate(), new_instance);
}

void ObjectTemplate::SetHandler(v8::NamedPropertyHandlerConfiguration const& handler) {
    // TODO: JerryScript: Add support for "interceptors"
}

/** SetAccessor: { value: 1556,
  writable: true,
  enumerable: true,
  configurable: true }
*/
void ObjectTemplate::SetAccessor(Local<String> name,
                                 AccessorGetterCallback getter,
                                 AccessorSetterCallback setter /* = 0 */,
                                 Local<Value> data /* = Local<Value>() */,
                                 AccessControl settings /* = DEFAULT */,
                                 PropertyAttribute attribute /* = None */,
                                 Local<AccessorSignature> signature /* = Local<AccessorSignature>() */) {
    JerryObjectTemplate* tmplt = reinterpret_cast<JerryObjectTemplate*>(this);

    JerryValue* jname = reinterpret_cast<JerryValue*>(*name)->Copy();
    JerryValue* jdata = NULL;
    if (!data.IsEmpty()) {
        jdata = reinterpret_cast<JerryValue*>(*data)->Copy();
    }

    tmplt->SetAccessor(jname, getter, setter, jdata, settings, attribute);
}

void ObjectTemplate::SetInternalFieldCount(int count) {
    JerryObjectTemplate* tmplt = reinterpret_cast<JerryObjectTemplate*>(this);
    tmplt->SetInteralFieldCount(count);
}

/* Signature */
Local<Signature> Signature::New(Isolate* isolate, Local<FunctionTemplate> receiver) {
    return Local<Signature>(reinterpret_cast<Signature*>(*receiver));
}

/* Exception & Error */
#define EXCEPTION_ERROR(error_class, error_type) \
    Local<Value> Exception::error_class(Local<String> message) { \
        JerryValue* jstr = reinterpret_cast<JerryValue*>(*message); \
        jerry_size_t req_sz = jerry_get_utf8_string_size(jstr->value()); \
        jerry_char_t str_buf_p[req_sz]; \
        jerry_string_to_utf8_char_buffer(jstr->value(), str_buf_p, req_sz); \
        jerry_value_t error_obj = jerry_create_error_sz (error_type, str_buf_p, req_sz); \
        RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(error_obj)); \
    }

EXCEPTION_ERROR(Error, JERRY_ERROR_COMMON);
EXCEPTION_ERROR(RangeError, JERRY_ERROR_RANGE);
EXCEPTION_ERROR(TypeError, JERRY_ERROR_TYPE);

/* StackFrame && StackTrace */
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

Local<StackTrace> StackTrace::CurrentStackTrace(v8::Isolate*, int, StackTrace::StackTraceOptions) {
    return Local<StackTrace>();
}

Local<StackFrame> StackTrace::GetFrame(unsigned int) const {
    return Local<StackFrame>();
}

/* HeapProfiler & HeapStatistics */
void HeapProfiler::SetWrapperClassInfoProvider(unsigned short class_id, WrapperInfoCallback cb) { }
void HeapProfiler::StartTrackingHeapObjects(bool track_allocations) { }

v8::HeapStatistics::HeapStatistics() { }

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

/* DebugBreak */
void Debug::DebugBreak(Isolate*) {
    // TODO: add trace
}

} // namespace v8
