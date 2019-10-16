#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v8env.h"

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

void method_named_getter(const v8::Local<v8::Name> propname, const v8::PropertyCallbackInfo<v8::Value>& info) {
    printf("Accessed named\n");
    info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), "ACCESSED", v8::NewStringType::kNormal).ToLocalChecked());
}

int main(int argc, char* argv[]) {
  // Initialize V8.
  V8Environment env(argc, argv);

  v8::Local<v8::External> external = v8::External::New(env.getIsolate(), (void*)22);

  printf("IsExt: %d\n", external->IsExternal());
  printf("IsObj: %d\n", external->IsObject());

  env.getContext()->Global()->Set(
      v8::String::NewFromUtf8(env.getIsolate(), "DATA"/*, v8::NewStringType::kNormal*/)/*.ToLocalChecked()*/,
      v8::Integer::New(env.getIsolate(), 33));

  v8::Local<v8::FunctionTemplate> functemp = v8::FunctionTemplate::New(env.getIsolate(), method_demo, external);
  functemp->Set(env.getIsolate(), "test", v8::Integer::New(env.getIsolate(), 38));

  v8::Local<v8::Function> funci = functemp->GetFunction();
  env.getContext()->Global()->Set(
      v8::String::NewFromUtf8(env.getIsolate(), "DEMO", v8::NewStringType::kNormal).ToLocalChecked(),
      funci);

  printf("HasInstance: %d\n", functemp->HasInstance(funci)); // Should be 0
  printf("HasInstance: %d\n", functemp->HasInstance(funci->NewInstance(0, NULL))); // Should be 1

  v8::Local<v8::FunctionTemplate> functemp2;
  //v8::Local<v8::FunctionTemplate> functemp2 = v8::FunctionTemplate::New(env.getIsolate(), method_constr);
  functemp2 = v8::FunctionTemplate::New(env.getIsolate(), method_constr);

  v8::Local<v8::FunctionTemplate> functemp3;
  { v8::HandleScope handle_scope(env.getIsolate());
  functemp3 = v8::FunctionTemplate::New(env.getIsolate(), method_constr);

  functemp2->SetAccessorProperty(
      v8::String::NewFromUtf8(env.getIsolate(), "DD", v8::NewStringType::kNormal).ToLocalChecked(),
      functemp3);
  }
  v8::Local<v8::ObjectTemplate> objt = functemp2->InstanceTemplate();
  objt->Set(env.getIsolate(), "test", v8::Integer::New(env.getIsolate(), 35));
  objt->SetAccessor(v8::String::NewFromUtf8(env.getIsolate(), "getprop", v8::NewStringType::kNormal).ToLocalChecked(),
                    method_getter);

  env.getContext()->Global()->Set(
      v8::String::NewFromUtf8(env.getIsolate(), "CONSTR2", v8::NewStringType::kNormal).ToLocalChecked(),
      functemp2->GetFunction());

  env.getContext()->Global()->Set(
      v8::String::NewFromUtf8(env.getIsolate(), "obj", v8::NewStringType::kNormal).ToLocalChecked(),
      objt->NewInstance(env.getContext()).ToLocalChecked());

  v8::Local<v8::ObjectTemplate> objt2 = v8::ObjectTemplate::New(env.getIsolate(), functemp2);
  objt2->SetAccessor(v8::String::NewFromUtf8(env.getIsolate(), "getprop", v8::NewStringType::kNormal).ToLocalChecked(),
                    method_getter);

  v8::Local<v8::Object> object_2 = objt2->NewInstance(env.getContext()).ToLocalChecked();
  object_2->SetAccessor(env.getContext(), v8::String::NewFromUtf8(env.getIsolate(), "getprop2", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Name>(),
                    method_named_getter);

  env.getContext()->Global()->Set(
      v8::String::NewFromUtf8(env.getIsolate(), "obj2", v8::NewStringType::kNormal).ToLocalChecked(),
      object_2);

  env.getContext()->Global()->Set(
      v8::String::NewFromUtf8(env.getIsolate(), "CONSTR", v8::NewStringType::kNormal).ToLocalChecked(),
      functemp2->GetFunction());

  // Create a string containing the JavaScript source code.
  v8::Local<v8::String> source = v8::String::NewFromUtf8(env.getIsolate(), R"(
      ('Hello' + ', World!' + DATA + DEMO(33, 44, 'txt') + DEMO.test + (new CONSTR()).test +
       obj.test + CONSTR2.DD + obj2.getprop + obj2.getprop2)
      )", v8::NewStringType::kNormal).ToLocalChecked();

  // Compile the source code.
  //v8::TryCatch tc;
  v8::Local<v8::Script> script =
      v8::Script::Compile(env.getContext(), source).ToLocalChecked();
  // Run the script to get the result.
  v8::Local<v8::Value> result = script->Run(env.getContext()).ToLocalChecked();
  // Convert the result to an UTF8 string and print it.
  v8::String::Utf8Value utf8(env.getIsolate(), result);
  printf("%s\n", *utf8);

  functemp2->GetFunction()->Call(v8::Undefined(env.getIsolate()), 0, NULL);

  // Dispose the env.getIsolate() and tear down V8.
  return 0;
}
