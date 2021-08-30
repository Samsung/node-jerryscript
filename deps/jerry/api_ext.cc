#include "v8.h"
#include "v8-profiler.h"
#include "jerryscript.h"

#include <signal.h>

#include "v8jerry_isolate.hpp"
#include "v8jerry_value.hpp"

// /* Remove the comments to enable trace macros */
// #define USE_TRACE

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

#define RETURN_HANDLE(T, ISOLATE, HANDLE) \
do { \
    return (HANDLE)->NewLocal<T>(ISOLATE); \
} while (0)

namespace i = v8::internal;

namespace v8 {

// Other functions

bool v8::Value::IsArgumentsObject() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_ARGUMENTS;
}

bool v8::Value::IsAsyncFunction() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_value_is_async_function (value);
}

bool v8::Value::IsBooleanObject() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_BOOLEAN;
}

bool v8::Value::IsBigIntObject() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_BIGINT;
}

bool v8::Value::IsDate() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_DATE;
}

bool v8::Value::IsGeneratorFunction() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_function_get_type (value) == JERRY_FUNCTION_TYPE_GENERATOR;
}

bool v8::Value::IsGeneratorObject() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_GENERATOR;
}

bool v8::Value::IsMap() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_container_type (value) == JERRY_CONTAINER_TYPE_MAP;
}

bool v8::Value::IsMapIterator() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_iterator_get_type (value) == JERRY_ITERATOR_TYPE_MAP;
}

bool v8::Value::IsNumberObject() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_NUMBER;
}

bool v8::Value::IsRegExp() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_REGEXP;
}

bool v8::Value::IsSet() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_container_type (value) == JERRY_CONTAINER_TYPE_SET;
}

bool v8::Value::IsSetIterator() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_iterator_get_type (value) == JERRY_ITERATOR_TYPE_SET;
}

bool v8::Value::IsStringObject() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_STRING;
}

bool v8::Value::IsSymbolObject() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_SYMBOL;
}

bool v8::Value::IsTypedArray() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_object_get_type (value) == JERRY_OBJECT_TYPE_TYPEDARRAY;
}

bool v8::Value::IsWeakMap() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_container_type (value) == JERRY_CONTAINER_TYPE_WEAKMAP;
}

bool v8::Value::IsWeakSet() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_container_type (value) == JERRY_CONTAINER_TYPE_WEAKSET;
}

bool v8::Value::IsInt8Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_INT8;
}

bool v8::Value::IsUint8Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_UINT8;
}

bool v8::Value::IsUint8ClampedArray() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_UINT8CLAMPED;
}

bool v8::Value::IsInt16Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_INT16;
}

bool v8::Value::IsUint16Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_UINT16;
}

bool v8::Value::IsInt32Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_INT32;
}

bool v8::Value::IsUint32Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_UINT32;
}

bool v8::Value::IsFloat32Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_FLOAT32;
}

bool v8::Value::IsFloat64Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_FLOAT64;
}

bool v8::Value::IsBigInt64Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_BIGINT64;
}

bool v8::Value::IsBigUint64Array() const {
  jerry_value_t value = reinterpret_cast<const JerryValue*> (this)->value();
  return jerry_get_typedarray_type (value) == JERRY_TYPEDARRAY_BIGUINT64;
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
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(BigInt64Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_BIGINT64));
}

Local<BigUint64Array> BigUint64Array::New(Local<ArrayBuffer> array_buffer,
                                          size_t byte_offset, size_t length) {
  V8_CALL_TRACE();
  JerryValue *jarray_buffer = reinterpret_cast<JerryValue*>(*array_buffer);
  RETURN_HANDLE(BigUint64Array, JerryIsolate::toV8(JerryIsolate::GetCurrent()), JerryValue::NewTypedArray(jarray_buffer, byte_offset, length, JERRY_TYPEDARRAY_BIGUINT64));
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
  V8_CALL_TRACE();
  jerry_value_t symbol = jerry_get_well_known_symbol (JERRY_SYMBOL_ITERATOR);

  RETURN_HANDLE(v8::Symbol, isolate, new JerryValue(symbol));
}

void Uint8Array::CheckCast(v8::Value* that) {
}

void Uint32Array::CheckCast(v8::Value* that) {
}
void Float64Array::CheckCast(v8::Value* that) {
}

void BigUint64Array::CheckCast(v8::Value* that) {
}

}
