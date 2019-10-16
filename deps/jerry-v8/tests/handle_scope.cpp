#include "v8env.h"
#include <cstring>
#include "assert.h"

static bool s_error_detected = false;

void error_handler(const char* location, const char* message) {
    // THIS must be called!
    s_error_detected = true;
    printf("Error reached, this is OK\n");

    exit(0);
}

v8::Local<v8::String> create_string(void) {
    v8::EscapableHandleScope scope(v8::Isolate::GetCurrent());
    v8::Local<v8::String> txt =
        v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "DATA", v8::NewStringType::kNormal).ToLocalChecked();

    return scope.Escape(txt);
}

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    v8::Local<v8::String> txt;
    {
        v8::HandleScope scope(env.getIsolate());
        txt = create_string();

        v8::String::Utf8Value utf8(env.getIsolate(), txt);
        ASSERT_STR_EQUAL(*utf8, "DATA");
        //printf("%s\n", *utf8);
    }

    {
        // "txt" is not valid here
        v8::String::Utf8Value utf8(env.getIsolate(), txt);
        ASSERT_EQUAL((uintptr_t)*utf8, (uintptr_t)0);
    }

    {
        v8::SealHandleScope sealedScope(env.getIsolate());
        {
            v8::HandleScope scope(env.getIsolate());
            v8::Local<v8::String> data = create_string();

            v8::String::Utf8Value utf8(env.getIsolate(), data);
            ASSERT_STR_EQUAL(*utf8, "DATA");
        }

        env.getIsolate()->SetFatalErrorHandler(error_handler);
        v8::Local<v8::String> data =
            v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "DATA2", v8::NewStringType::kNormal).ToLocalChecked();
        // UNREACHABLE!!!!
    }

    return 0;
}
