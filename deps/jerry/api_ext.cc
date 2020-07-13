#include "v8.h"
#include "v8-profiler.h"
#include "jerryscript.h"

#include <signal.h>

#define UNIMPLEMENTED(line) abort()

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
  UNIMPLEMENTED(-1);
  return Local<Int8Array>();
}

Local<Uint8Array> Uint8Array::New(Local<ArrayBuffer> array_buffer,
                                  size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Uint8Array>();
}

Local<Uint8ClampedArray> Uint8ClampedArray::New(Local<ArrayBuffer> array_buffer,
                                                size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Uint8ClampedArray>();
}

Local<Int16Array> Int16Array::New(Local<ArrayBuffer> array_buffer,
                                  size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Int16Array>();
}

Local<Uint16Array> Uint16Array::New(Local<ArrayBuffer> array_buffer,
                                    size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Uint16Array>();
}

Local<Int32Array> Int32Array::New(Local<ArrayBuffer> array_buffer,
                                  size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Int32Array>();
}

Local<Uint32Array> Uint32Array::New(Local<ArrayBuffer> array_buffer,
                                    size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Uint32Array>();
}

Local<BigInt64Array> BigInt64Array::New(Local<ArrayBuffer> array_buffer,
                                        size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<BigInt64Array>();
}

Local<BigUint64Array> BigUint64Array::New(Local<ArrayBuffer> array_buffer,
                                          size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<BigUint64Array>();
}

Local<Float32Array> Float32Array::New(Local<ArrayBuffer> array_buffer,
                                      size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Float32Array>();
}

Local<Float64Array> Float64Array::New(Local<ArrayBuffer> array_buffer,
                                      size_t byte_offset, size_t length) {
  UNIMPLEMENTED(-1);
  return Local<Float64Array>();
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
