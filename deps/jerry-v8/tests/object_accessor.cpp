#include "v8env.h"
#include "assert.h"

static int32_t countDemoAccess = 0xFFFFFFFF;

static void demoObjectPropGetter(v8::Local<v8::Name> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::String::Utf8Value propStr(info.GetIsolate(), property->ToString());
    printf("DemoObject: Getter called for Property: %s\n", *propStr);

    if (strncmp("propA", *propStr, 6) == 0) {
        countDemoAccess += 1;
    }
}

static void demoObjectPropSetter(v8::Local<v8::Name> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
    v8::String::Utf8Value propStr(info.GetIsolate(), property->ToString());
    printf("DemoObject: Setter called for Property: %s\n", *propStr);

    if (strncmp("propA", *propStr, 6) == 0) {
        countDemoAccess += 100;
    }
}

void testObject(int argc, char** argv) {
    countDemoAccess = 0;

    // Initialize V8.
    V8Environment env(argc, argv);

    v8::Local<v8::Object> demoObject = v8::Object::New(env.getIsolate());

    demoObject->SetAccessor(
        env.getContext(),
        v8::String::NewFromUtf8(env.getIsolate(), "propA", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Name>(),
        demoObjectPropGetter,
        demoObjectPropSetter);

    env.getContext()->Global()->Set(
        env.getContext(),
        v8::String::NewFromUtf8(env.getIsolate(), "demo", v8::NewStringType::kNormal).ToLocalChecked(),
        demoObject);


    v8::Local<v8::String> source = v8::String::NewFromUtf8(
        env.getIsolate(),
        R"( demo.PropA; demo.propA = 7; demo.propA;
        )", v8::NewStringType::kNormal).ToLocalChecked();

    v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();
    v8::Local<v8::Value> result = script->Run(env.getContext()).ToLocalChecked();

    ASSERT_EQUAL(result->IsUndefined(), true);
    ASSERT_EQUAL(countDemoAccess, 101);
}

static void demoTemplatePropGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
    v8::String::Utf8Value propStr(info.GetIsolate(), property);
    printf("DemoTemplate: Getter called for Property: %s\n", *propStr);

    if (strncmp("propA", *propStr, 6) == 0) {
        countDemoAccess += 2;
    }
}

static void demoTemplatePropSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info) {
    v8::String::Utf8Value propStr(info.GetIsolate(), property);
    printf("DemoTemplate: Setter called for Property: %s\n", *propStr);

    if (strncmp("propA", *propStr, 6) == 0) {
        countDemoAccess += 200;
    }
}


void testObjectTemplate(int argc, char** argv) {
    countDemoAccess = 0;

    // Initialize V8.
    V8Environment env(argc, argv);

    v8::Local<v8::ObjectTemplate> demoTemplate = v8::ObjectTemplate::New(env.getIsolate());

    demoTemplate->SetAccessor(
        v8::String::NewFromUtf8(env.getIsolate(), "propA", v8::NewStringType::kNormal).ToLocalChecked(),
        demoTemplatePropGetter,
        demoTemplatePropSetter);

    env.getContext()->Global()->Set(
        env.getContext(),
        v8::String::NewFromUtf8(env.getIsolate(), "demo", v8::NewStringType::kNormal).ToLocalChecked(),
        demoTemplate->NewInstance(env.getContext()).ToLocalChecked());


    v8::Local<v8::String> source = v8::String::NewFromUtf8(
        env.getIsolate(),
        R"( demo.PropA; demo.propA = 7; demo.propA;
        )", v8::NewStringType::kNormal).ToLocalChecked();

    v8::Local<v8::Script> script = v8::Script::Compile(env.getContext(), source).ToLocalChecked();
    v8::Local<v8::Value> result = script->Run(env.getContext()).ToLocalChecked();

    ASSERT_EQUAL(result->IsUndefined(), true);
    ASSERT_EQUAL(countDemoAccess, 202);
}

int main(int argc, char** argv) {

    testObject(argc, argv);
    testObjectTemplate(argc, argv);

    return 0;
}
