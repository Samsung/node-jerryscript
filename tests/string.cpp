#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v8env.h"
#include "assert.h"

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    char buffer[100];
    char helloworldbuffer[] = "Hello world!";

    // Hungarian special letters: áéíóőúű.
    v8::Local<v8::String> hun = v8::String::NewFromUtf8(env.getIsolate(), "\xc3\xa1\xc3\xa9\xc3\xad\xc3\xb3\xc5\x91\xc3\xba\xc5\xb1", v8::NewStringType::kNormal).ToLocalChecked();

    // Strings for Hello World!
    v8::Local<v8::String> hello = v8::String::NewFromUtf8(env.getIsolate(), "Hello ", v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::String> world = v8::String::NewFromUtf8(env.getIsolate(), "World!", v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::String> helloworld = v8::String::Concat(hello, world);

    // ASSERT_EQUAL(hello->IsString(), true);  // Fix it! v8.h defines an IsString method.
    ASSERT_EQUAL(hello->Length(), world->Length());
    ASSERT_EQUAL(hello->Utf8Length(), world->Utf8Length());
    ASSERT_EQUAL(hun->Length(), hun->Utf8Length());
    ASSERT_EQUAL(helloworld->Length(), hello->Length() + world->Length());

    helloworld->WriteUtf8(buffer, helloworld->Length(), NULL, 0);
    ASSERT_STR_EQUAL(buffer, "Hello World!");

    v8::String::Utf8Value utf8(env.getIsolate(), helloworld);
    ASSERT_STR_EQUAL(buffer, *utf8);

    return 0;
}