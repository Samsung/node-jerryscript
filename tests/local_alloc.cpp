#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v8env.h"
#include "assert.h"

static const char static_data[] = "Hello";

static struct : public v8::String::ExternalOneByteStringResource {
    const char* data() const override {
        return reinterpret_cast<const char*>(static_data);
    }
    size_t length() const override { return strlen((const char*)static_data); }
    void Dispose() override { /* Default calls `delete this`. */ printf("Clearing ext data str\n"); }

    v8::Local<v8::String> ToStringChecked(v8::Isolate* isolate) {
        return v8::String::NewExternalOneByte(isolate, this).ToLocalChecked();
    }
} ext_data;


static const char static_key[] = "Key";

static struct : public v8::String::ExternalOneByteStringResource {
    const char* data() const override {
        return reinterpret_cast<const char*>(static_key);
    }
    size_t length() const override { return strlen((const char*)static_key); }
    void Dispose() override { /* Default calls `delete this`. */ printf("Clearing ext key str\n"); }

    v8::Local<v8::String> ToStringChecked(v8::Isolate* isolate) {
        return v8::String::NewExternalOneByte(isolate, this).ToLocalChecked();
    }
} ext_key;

inline v8::Local<v8::String> OneByteString(v8::Isolate* isolate,
                                           const char* data,
                                           int length) {
  return v8::String::NewFromOneByte(isolate,
                                    reinterpret_cast<const uint8_t*>(data),
                                    v8::NewStringType::kNormal,
                                    length).ToLocalChecked();
}


void fill_object(v8::Local<v8::Object> target) {
    target->Set(target->CreationContext(),
                OneByteString(target->GetIsolate(), "mydata", 6),
                v8::Integer::New(target->GetIsolate(), 33));

    target->Set(target->CreationContext(),
                ext_key.ToStringChecked(target->GetIsolate()),
                ext_data.ToStringChecked(target->GetIsolate())).FromJust();
}

void init_object(const v8::FunctionCallbackInfo<v8::Value>& info) {
    v8::Local<v8::Object> target = v8::Object::New(info.GetIsolate());
    fill_object(target);

    info.GetReturnValue().Set(target);
}

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    v8::Local<v8::Object> global = env.getContext()->Global();

    {
        v8::Local<v8::Object> global = env.getContext()->Global();
        global->Set(OneByteString(env.getIsolate(), "global", 6), global);
    }

    v8::Persistent<v8::Object> cache;

    {
        v8::HandleScope handle(env.getIsolate());
        v8::Local<v8::FunctionTemplate> subfunc = v8::FunctionTemplate::New(env.getIsolate(), init_object);

        v8::Local<v8::Object> obj = v8::Object::New(env.getIsolate());
        cache.Reset(env.getIsolate(), obj);
        v8::Local<v8::Value> args[] = {
            cache.Get(env.getIsolate()),
        };

        v8::Local<v8::Value> result = subfunc->GetFunction()->Call(v8::Undefined(env.getIsolate()), 1, args);

        cache.Get(env.getIsolate())->Set(OneByteString(env.getIsolate(), "OBJ1", 4), result);
    }
    {
        v8::HandleScope handle_scope(env.getIsolate());

        v8::Local<v8::Value> obj = cache.Get(env.getIsolate())->Get(env.getContext(), OneByteString(env.getIsolate(), "OBJ1", 4)).ToLocalChecked();

        v8::Local<v8::String> str = obj.As<v8::Object>()->Get(env.getContext(), OneByteString(env.getIsolate(), "Key", 3))
                            .ToLocalChecked().As<v8::String>();
        v8::String::Utf8Value utf8(env.getIsolate(), str);
        printf("ext_data: %s\n", *utf8);

    }

    cache.Reset();

    return 0;
}
