#include "v8env.h"
#include "assert.h"

static int message_counter;

static void on_message(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
    message_counter++;
    if (error->IsObject()) {
        message_counter++;
    }

    v8::HandleScope scope(v8::Isolate::GetCurrent());

    v8::Local<v8::Object> obj = error.As<v8::Object>();
    v8::Local<v8::String> stackString = v8::String::NewFromUtf8(scope.GetIsolate(), "stack", v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::Value> stackValue = obj->Get(stackString);
    if (stackValue->IsString()) {
        message_counter++;
    }
}

int main(int argc, char* argv[]) {
    message_counter = 0;
    // Initialize V8.
    V8Environment env(argc, argv);

    env.getIsolate()->AddMessageListener(on_message);

    {
        v8::TryCatch tc(env.getIsolate());
        tc.SetVerbose(true);

        v8::Local<v8::String> source =
            v8::String::NewFromUtf8(env.getIsolate(), "throw new Error('Bla');", v8::NewStringType::kNormal).ToLocalChecked();

        v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();

        ASSERT_EQUAL(tc.HasCaught(), false);
        ASSERT_EQUAL(message_counter, 0);
        v8::MaybeLocal<v8::Value> result = script->Run(env.getContext());
        ASSERT_EQUAL(tc.HasCaught(), true);
    }
    ASSERT_EQUAL(message_counter, 3);

    return 0;
}
