#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libplatform/libplatform.h"
#include "v8.h"

void method_demo(const v8::FunctionCallbackInfo<v8::Value>& info) {
    printf("Called with: %d\n", info.Length());

    v8::Local<v8::Value> arg = info[0];

    printf("Arg0: %d\n", arg->Int32Value());
    printf("Arg1: %d\n", info[1]->Int32Value());

    v8::String::Utf8Value utf8(info.GetIsolate(), info[2].As<v8::String>());
    printf("Arg2: %s\n", *utf8);

    v8::Local<v8::String> txt =
        v8::String::NewFromUtf8(info.GetIsolate(), "DATA", v8::NewStringType::kNormal).ToLocalChecked();

    v8::ReturnValue<v8::Value> retVal = info.GetReturnValue();
    retVal.Set(txt);
}

int main(int argc, char* argv[]) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  //std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  std::unique_ptr<v8::Platform> platform(v8::platform::CreateDefaultPlatform());
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);
    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);
    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    // Enter the context for compiling and running the hello world script.

    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "DATA", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::Integer::New(isolate, 33));

    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "DEMO", v8::NewStringType::kNormal).ToLocalChecked(),
        v8::FunctionTemplate::New(isolate, method_demo)->GetFunction());


    v8::Context::Scope context_scope(context);
    // Create a string containing the JavaScript source code.
    v8::Local<v8::String> source =
        v8::String::NewFromUtf8(isolate, "'Hello' + ', World!' + DATA + DEMO(33, 44, 'txt')",
                                v8::NewStringType::kNormal)
            .ToLocalChecked();
    // Compile the source code.
    v8::Local<v8::Script> script =
        v8::Script::Compile(context, source).ToLocalChecked();
    // Run the script to get the result.
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
    // Convert the result to an UTF8 string and print it.
    v8::String::Utf8Value utf8(isolate, result);
    printf("%s\n", *utf8);
  }
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}
