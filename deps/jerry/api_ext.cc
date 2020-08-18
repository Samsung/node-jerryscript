#include "v8.h"
#include "v8-profiler.h"
#include "jerryscript.h"

#include <signal.h>

#include "v8jerry_isolate.hpp"
#include "v8jerry_value.hpp"

// /* Remove the comments to enable trace macros */
#define USE_TRACE

#if defined(USE_TRACE)
#include <iostream>
#include <sstream>

#define V8_CALL_TRACE() do { _LogTrace(__LINE__, __FILE__, __PRETTY_FUNCTION__); } while (0)

static void _LogTrace(int line, std::string file_name, std::string func_name) {
    size_t last_separator_pos = file_name.find_last_of("/");

    std::ostringstream stream;
    stream << "[Trace]: " << file_name.substr(last_separator_pos + 1) << "(" << line << ") : ";
    stream << func_name << std::endl;

    std::cerr << stream.str();
}

#else
#define V8_CALL_TRACE()
#endif

#define UNIMPLEMENTED(line) abort()

#define RETURN_HANDLE(T, ISOLATE, HANDLE) \
do {                                                                    \
    JerryHandle *__handle = HANDLE;                                    \
    return v8::Local<T>::New(ISOLATE, reinterpret_cast<T*>(__handle)); \
} while (0)

namespace i = v8::internal;

namespace v8 {

// Other functions

bool v8::Value::IsArgumentsObject() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsAsyncFunction() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsBooleanObject() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsBigIntObject() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsDate() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsGeneratorFunction() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsGeneratorObject() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsMap() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsMapIterator() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsNumberObject() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsRegExp() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsSharedArrayBuffer() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsSet() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsSetIterator() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsStringObject() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsSymbolObject() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsTypedArray() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsWeakMap() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsWeakSet() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsInt8Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsUint8Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsUint8ClampedArray() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsInt16Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsUint16Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsInt32Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsUint32Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsFloat32Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsFloat64Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsBigInt64Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

bool v8::Value::IsBigUint64Array() const {
  UNIMPLEMENTED(-1);
  return false;
}

Local<Int8Array> Int8Array::New(Local<ArrayBuffer> array_buffer,
                                size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Int8Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_INT8));
}

Local<Uint8Array> Uint8Array::New(Local<ArrayBuffer> array_buffer,
                                  size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Uint8Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_UINT8));
}

Local<Uint8ClampedArray> Uint8ClampedArray::New(Local<ArrayBuffer> array_buffer,
                                                size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Uint8ClampedArray, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_UINT8CLAMPED));
}

Local<Int16Array> Int16Array::New(Local<ArrayBuffer> array_buffer,
                                  size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Int16Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_INT16));
}

Local<Uint16Array> Uint16Array::New(Local<ArrayBuffer> array_buffer,
                                    size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Uint16Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_UINT16));
}

Local<Int32Array> Int32Array::New(Local<ArrayBuffer> array_buffer,
                                  size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Int32Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_INT32));
}

Local<Uint32Array> Uint32Array::New(Local<ArrayBuffer> array_buffer,
                                    size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Uint32Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_UINT32));
}

Local<BigInt64Array> BigInt64Array::New(Local<ArrayBuffer> array_buffer,
                                        size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<BigInt64Array>();
}

Local<BigUint64Array> BigUint64Array::New(Local<ArrayBuffer> array_buffer,
                                          size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(BigUint64Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_FLOAT64));
}

Local<Float32Array> Float32Array::New(Local<ArrayBuffer> array_buffer,
                                      size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Float32Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_FLOAT32));
}

Local<Float64Array> Float64Array::New(Local<ArrayBuffer> array_buffer,
                                      size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(Float64Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_FLOAT64));
}

Local<v8::Symbol> Symbol::GetIterator(Isolate* isolate) {
  UNIMPLEMENTED(-1);
  return Local<v8::Symbol>();
}

void Uint8Array::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(-1);
}

void Uint32Array::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(-1);
}
void Float64Array::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(-1);
}

void BigUint64Array::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(-1);
}

}
