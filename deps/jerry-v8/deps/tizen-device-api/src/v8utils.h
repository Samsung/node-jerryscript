#ifndef TIZEN_DEVICE_API_V8UTILS_H_
#define TIZEN_DEVICE_API_V8UTILS_H_

#include "v8.h"

namespace v8utils {
static inline v8::Local<v8::Value> ToNum(v8::Local<v8::Context> context,
                                          double x) {
  return v8::Number::New(context->GetIsolate(), x);
}

static inline v8::Local<v8::String> ToStr(v8::Local<v8::Context> context,
                                           const char* x) {
  return v8::String::NewFromUtf8(context->GetIsolate(), x,
                                 v8::NewStringType::kNormal)
      .ToLocalChecked();
}

static inline v8::Local<v8::String> ToStrInternal(v8::Local<v8::Context> context,
                                           const char* x) {
  return v8::String::NewFromUtf8(context->GetIsolate(), x,
                                 v8::NewStringType::kInternalized)
      .ToLocalChecked();
}

static inline v8::Local<v8::Script> Compile(v8::Local<v8::Context> context,
                                               v8::Local<v8::String> x) {
  v8::Local<v8::Script> result;
  if (v8::Script::Compile(context, x).ToLocal(&result)) {
    return result;
  }
  return v8::Local<v8::Script>();
}

static inline v8::Local<v8::Script> Compile(v8::Local<v8::Context> context,
                                               const char* x) {
  return Compile(context, ToStr(context, x));
}

static inline v8::Local<v8::Value> CompileRun(v8::Local<v8::Context> context,
                                              v8::Local<v8::String> source) {
  v8::Local<v8::Value> result;
  if (Compile(context, source)->Run(context).ToLocal(&result)) {
    return result;
  }
  return v8::Local<v8::Value>();
}

static inline v8::Local<v8::Value> CompileRun(v8::Local<v8::Context> context,
                                              const char* source) {
  return CompileRun(context, ToStr(context, source));
}
}
#endif
