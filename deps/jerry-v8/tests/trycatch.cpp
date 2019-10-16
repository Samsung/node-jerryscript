#include "v8env.h"
#include "assert.h"

enum {
    NO_ERROR = 11,
    DO_ERROR = 99,
};

#define ERROR_MESSAGE "Created ERROR"

void method_demo(const v8::FunctionCallbackInfo<v8::Value>& info) {
    switch(info[0]->Int32Value()) {
        case NO_ERROR: info.GetReturnValue().Set(v8::Integer::New(info.GetIsolate(), NO_ERROR + 5)); break;
        case DO_ERROR:
            info.GetIsolate()->ThrowException(
                    v8::String::NewFromUtf8(info.GetIsolate(), ERROR_MESSAGE, v8::NewStringType::kNormal).ToLocalChecked()); break;
    }
}

void method_getter(const v8::Local<v8::Name> propname, const v8::PropertyCallbackInfo<v8::Value>& info) {
    info.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(info.GetIsolate(), ERROR_MESSAGE, v8::NewStringType::kNormal).ToLocalChecked());
}

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    // Test simple throw in eval
    {
        v8::Local<v8::String> source =
            v8::String::NewFromUtf8(env.getIsolate(), "throw new Error('Bla');", v8::NewStringType::kNormal).ToLocalChecked();

        v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();

        v8::TryCatch tc(env.getIsolate());
        ASSERT_EQUAL(tc.HasCaught(), false);
        v8::MaybeLocal<v8::Value> result = script->Run(env.getContext());
        ASSERT_EQUAL(tc.HasCaught(), true);
    }

    // Test TryCatch.Rethrow()
    {
        v8::Local<v8::String> source =
            v8::String::NewFromUtf8(env.getIsolate(), "throw new Error('Bla');", v8::NewStringType::kNormal).ToLocalChecked();

        v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();

        v8::TryCatch tc(env.getIsolate());
        ASSERT_EQUAL(tc.HasCaught(), false);
        {
            v8::TryCatch tc_inner(env.getIsolate());
            ASSERT_EQUAL(tc_inner.HasCaught(), false);
            v8::MaybeLocal<v8::Value> result = script->Run(env.getContext());
            ASSERT_EQUAL(tc_inner.HasCaught(), true);
            if (tc_inner.HasCaught()) {
                // got an exception rethrow it.
                tc_inner.ReThrow();
            }
        }

        ASSERT_EQUAL(tc.HasCaught(), true);
    }

    // Test function call which throws error.
    {
        v8::TryCatch tc(env.getIsolate());
        v8::Local<v8::Function> func = v8::Function::New(env.getContext(), method_demo).ToLocalChecked();

        {
            v8::Local<v8::Value> args[] = {
                v8::Integer::New(env.getIsolate(), 11),
            };
            v8::Local<v8::Value> result = func->Call(v8::Undefined(env.getIsolate()), 1, args);
            ASSERT_EQUAL(tc.HasCaught(), false);
            ASSERT_EQUAL(result->Int32Value(), NO_ERROR + 5);
        }

        {
            v8::Local<v8::Value> args[] = {
                v8::Integer::New(env.getIsolate(), DO_ERROR),
            };
            v8::Local<v8::Value> result = func->Call(v8::Undefined(env.getIsolate()), 1, args);
            ASSERT_EQUAL(tc.HasCaught(), true);
            ASSERT_EQUAL(result.IsEmpty(), true);
        }
    }

    // Test object getter which throws error
    {
        v8::TryCatch tc(env.getIsolate());

        v8::Local<v8::String> key = v8::String::NewFromUtf8(env.getIsolate(), "errorprop", v8::NewStringType::kNormal).ToLocalChecked();

        v8::Local<v8::Object> obj = v8::Object::New(env.getIsolate());
        {
            v8::Maybe<bool> set_result =
                obj->SetAccessor(env.getContext(), key.As<v8::Name>(), method_getter);
            ASSERT_EQUAL(set_result.IsJust(), true);
            ASSERT_EQUAL(set_result.ToChecked(), true);
            ASSERT_EQUAL(tc.HasCaught(), false);
        }

        {
            v8::MaybeLocal<v8::Value> get_result = obj->Get(env.getContext(), key);
            ASSERT_EQUAL(get_result.IsEmpty(), true);
            ASSERT_EQUAL(tc.HasCaught(), true);
            {
                v8::Local<v8::Value> ex = tc.Exception();
                ASSERT_EQUAL(ex.IsEmpty(), false);
                v8::MaybeLocal<v8::String> ex_msg = ex->ToString(env.getContext());
                v8::String::Utf8Value ex_msg_utf8(env.getIsolate(), ex_msg.ToLocalChecked());
                ASSERT_STR_EQUAL(*ex_msg_utf8, ERROR_MESSAGE);
            }
        }

        tc.Reset();
        ASSERT_EQUAL(tc.HasCaught(), false);

        {
            v8::Maybe<bool> set_result = env.getContext()->Global()->Set(
                env.getContext(),
                v8::String::NewFromUtf8(env.getIsolate(), "err_obj", v8::NewStringType::kNormal).ToLocalChecked(),
                obj);
            ASSERT_EQUAL(set_result.IsJust(), true);
            ASSERT_EQUAL(set_result.ToChecked(), true);
            ASSERT_EQUAL(tc.HasCaught(), false);
        }

        v8::Local<v8::String> source =
            v8::String::NewFromUtf8(env.getIsolate(), "err_obj.errorprop", v8::NewStringType::kNormal).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();
        v8::MaybeLocal<v8::Value> eval_result = script->Run(env.getContext());
        ASSERT_EQUAL(tc.HasCaught(), true);
        ASSERT_EQUAL(eval_result.IsEmpty(), true);

        v8::Local<v8::Value> ex = tc.Exception();
        ASSERT_EQUAL(ex.IsEmpty(), false);
        {
            v8::MaybeLocal<v8::String> ex_msg = ex->ToString(env.getContext());
            v8::String::Utf8Value ex_msg_utf8(env.getIsolate(), ex_msg.ToLocalChecked());
            ASSERT_STR_EQUAL(*ex_msg_utf8, ERROR_MESSAGE);
        }
    }

    return 0;
}
