#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libplatform/libplatform.h"
#include "v8.h"

void method_demo(const v8::FunctionCallbackInfo<v8::Value>& info) {
    printf("Called with: %d\n", info.Length());
    v8::Local<v8::Value> data = info.Data();
    printf("Data is obj: %d\n", data->IsObject());
    printf("Data is external: %d\n", data->IsExternal());

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

void method_constr(const v8::FunctionCallbackInfo<v8::Value>& info) {
    printf("Construct\n");
}

void method_getter(const v8::Local<v8::String> propname, const v8::PropertyCallbackInfo<v8::Value>& info) {
    printf("Accessed\n");
    info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), "ACCESSED", v8::NewStringType::kNormal).ToLocalChecked());
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
    // This must be before setting any context related global properties!
    v8::Context::Scope context_scope(context);

    v8::Local<v8::External> external = v8::External::New(isolate, (void*)22);

    printf("IsExt: %d\n", external->IsExternal());
    printf("IsObj: %d\n", external->IsObject());

    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "DATA"/*, v8::NewStringType::kNormal*/)/*.ToLocalChecked()*/,
        v8::Integer::New(isolate, 33));

    v8::Local<v8::FunctionTemplate> functemp = v8::FunctionTemplate::New(isolate, method_demo, external);
    functemp->Set(isolate, "test", v8::Integer::New(isolate, 38));

    v8::Local<v8::Function> funci = functemp->GetFunction();
    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "DEMO", v8::NewStringType::kNormal).ToLocalChecked(),
        funci);

    printf("HasInstance: %d\n", functemp->HasInstance(funci)); // Should be 0
    printf("HasInstance: %d\n", functemp->HasInstance(funci->NewInstance(0, NULL))); // Should be 1

    v8::Local<v8::FunctionTemplate> functemp2;
    //v8::Local<v8::FunctionTemplate> functemp2 = v8::FunctionTemplate::New(isolate, method_constr);
    functemp2 = v8::FunctionTemplate::New(isolate, method_constr);

    v8::Local<v8::FunctionTemplate> functemp3;
    { v8::HandleScope handle_scope(isolate);
    functemp3 = v8::FunctionTemplate::New(isolate, method_constr);

    functemp2->SetAccessorProperty(
        v8::String::NewFromUtf8(isolate, "DD", v8::NewStringType::kNormal).ToLocalChecked(),
        functemp3);
    }
    v8::Local<v8::ObjectTemplate> objt = functemp2->InstanceTemplate();
    objt->Set(isolate, "test", v8::Integer::New(isolate, 35));
    objt->SetAccessor(v8::String::NewFromUtf8(isolate, "getprop", v8::NewStringType::kNormal).ToLocalChecked(),
                      method_getter);

    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "CONSTR2", v8::NewStringType::kNormal).ToLocalChecked(),
        functemp2->GetFunction());

    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "obj", v8::NewStringType::kNormal).ToLocalChecked(),
        objt->NewInstance(context).ToLocalChecked());

    v8::Local<v8::ObjectTemplate> objt2 = v8::ObjectTemplate::New(isolate, functemp2);
    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "obj2", v8::NewStringType::kNormal).ToLocalChecked(),
        objt2->NewInstance(context).ToLocalChecked());

    context->Global()->Set(
        v8::String::NewFromUtf8(isolate, "CONSTR", v8::NewStringType::kNormal).ToLocalChecked(),
        functemp2->GetFunction());

    // Create a string containing the JavaScript source code.
    v8::Local<v8::String> source =
        v8::String::NewFromUtf8(isolate, "'Hello' + ', World!' + DATA + DEMO(33, 44, 'txt') + DEMO.test + (new CONSTR()).test + obj.test + CONSTR2.DD + obj.getprop",
                                v8::NewStringType::kNormal)
            .ToLocalChecked();
    // Compile the source code.
    //v8::TryCatch tc;
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
