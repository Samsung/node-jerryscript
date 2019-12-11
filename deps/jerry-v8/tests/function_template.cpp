#include "v8env.h"
#include "assert.h"

void method_demo(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), "ACCESSED"));
}

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    v8::Local<v8::String> protoString = v8::String::NewFromUtf8(env.getIsolate(), "prototype");
    v8::Local<v8::String> getPropOkString = v8::String::NewFromUtf8(env.getIsolate(), "getpropOk");
    v8::Local<v8::String> getPropErrorString = v8::String::NewFromUtf8(env.getIsolate(), "getpropError");

    v8::Local<v8::FunctionTemplate> tmplt = v8::FunctionTemplate::New(env.getIsolate());
    {
        v8::Local<v8::ObjectTemplate> prototype = tmplt->PrototypeTemplate();

        prototype->SetInternalFieldCount(1);

        v8::Local<v8::String> nameKey = v8::String::NewFromUtf8(env.getIsolate(), "Foo");
        v8::Local<v8::String> demoString = v8::String::NewFromUtf8(env.getIsolate(), "demo");
        prototype->Set(nameKey, demoString);

        // By configuring the signature for a template method it should throw an error if an incorrect "this" value is used during access.
        {
            v8::Local<v8::Signature> signature = v8::Signature::New(env.getIsolate(), tmplt);
            v8::Local<v8::FunctionTemplate> getprop_method =
                v8::FunctionTemplate::New(env.getIsolate(), method_demo, v8::Local<v8::External>(), signature);

            prototype->SetAccessorProperty(getPropErrorString, getprop_method, v8::Local<v8::FunctionTemplate>());
        }

        // Without a signature the template method should work in every case
        {
            v8::Local<v8::FunctionTemplate> getprop_method = v8::FunctionTemplate::New(env.getIsolate(), method_demo);
            prototype->SetAccessorProperty(getPropOkString, getprop_method, v8::Local<v8::FunctionTemplate>());
        }
    }

    v8::Local<v8::Function> func = tmplt->GetFunction();
    {
        env.getContext()->Global()->Set(
            v8::String::NewFromUtf8(env.getIsolate(), "DemoFunc"),
            func);
    }

    ASSERT_EQUAL(func->InternalFieldCount(), 0);

    v8::Local<v8::Value> prototypeVal = func->Get(protoString);
    ASSERT_EQUAL(prototypeVal->IsObject(), true);
    v8::Local<v8::Object> prototype = prototypeVal.As<v8::Object>();
    ASSERT_EQUAL(prototype->InternalFieldCount(), 1);

    // Test the "prototype.getpropError" via object.get
    {
        v8::TryCatch tc(env.getIsolate());

        v8::Local<v8::Value> result = prototype->Get(v8::String::NewFromUtf8(env.getIsolate(), "getpropError"));
        ASSERT_EQUAL(result.IsEmpty(), true);
        ASSERT_EQUAL(tc.HasCaught(), true);
    }

    // Test the "prototype.getpropError" via JS code
    {
        v8::TryCatch tc(env.getIsolate());

        v8::Local<v8::String> source = v8::String::NewFromUtf8(env.getIsolate(), "DemoFunc.prototype.getpropError");
        v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();
        v8::MaybeLocal<v8::Value> result = script->Run(env.getContext());
        ASSERT_EQUAL(result.IsEmpty(), true);
        ASSERT_EQUAL(tc.HasCaught(), true);
    }

    // Test "instance.getpropError" via object.get should not throw error
    {
        v8::TryCatch tc(env.getIsolate());

        v8::Local<v8::Object> instance = func->NewInstance(0, NULL);
        v8::Local<v8::Value> result = instance->Get(v8::String::NewFromUtf8(env.getIsolate(), "getpropError"));
        ASSERT_EQUAL(result.IsEmpty(), false);
        ASSERT_EQUAL(result->IsString(), true);
        ASSERT_EQUAL(tc.HasCaught(), false);

        v8::String::Utf8Value utf8(env.getIsolate(), result.As<v8::String>());
        ASSERT_STR_EQUAL("ACCESSED", *utf8);
    }

    // Test "instance.getpropError" via JS code should not throw error
    {
        v8::TryCatch tc(env.getIsolate());

        v8::Local<v8::String> source = v8::String::NewFromUtf8(env.getIsolate(), "(new DemoFunc).getpropError");
        v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();
        v8::MaybeLocal<v8::Value> eval_result = script->Run(env.getContext());
        ASSERT_EQUAL(eval_result.IsEmpty(), false);

        v8::Local<v8::Value> result = eval_result.ToLocalChecked();
        ASSERT_EQUAL(result->IsString(), true);
        ASSERT_EQUAL(tc.HasCaught(), false);

        v8::String::Utf8Value utf8(env.getIsolate(), result.As<v8::String>());
        ASSERT_STR_EQUAL("ACCESSED", *utf8);
    }

    // Access elements from prototype "(new DemoFunc).Foo"
    {
        v8::TryCatch tc(env.getIsolate());

        v8::Local<v8::Object> instance = func->NewInstance(0, NULL);
        v8::Local<v8::Value> result = instance->Get(v8::String::NewFromUtf8(env.getIsolate(), "Foo"));
        ASSERT_EQUAL(result.IsEmpty(), false);
        ASSERT_EQUAL(result->IsString(), true);
        ASSERT_EQUAL(tc.HasCaught(), false);

        v8::String::Utf8Value utf8(env.getIsolate(), result.As<v8::String>());
        ASSERT_STR_EQUAL("demo", *utf8);
    }

    return 0;
}
