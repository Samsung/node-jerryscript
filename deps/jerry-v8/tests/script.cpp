#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v8env.h"
#include "assert.h"

// JavaScript source code.
const char jsSource[] = "               \
    a = 2;                              \
    b = 'Hello Script execution -> ';   \
                                        \
    for (i = 0; i < 200; i++) {         \
        a += 1;                         \
    }                                   \
                                        \
    b + a;                              \
";

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    v8::Local<v8::String> source = v8::String::NewFromUtf8(env.getIsolate(), jsSource, v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();
    v8::Local<v8::Value> result = script->Run(env.getContext()).ToLocalChecked();

    v8::String::Utf8Value utf8(env.getIsolate(), result);
    ASSERT_STR_EQUAL(*utf8, "Hello Script execution -> 202");

    return 0;
}