#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v8env.h"
#include "assert.h"

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    // Check if the Math built-in available via global.
    v8::Local<v8::Object> global = env.getContext()->Global();
    v8::Local<v8::String> math = v8::String::NewFromUtf8(env.getIsolate(), "Math", v8::NewStringType::kNormal).ToLocalChecked();
    v8::Maybe<bool> hasMath = global->Has(env.getContext(), math);

    ASSERT_EQUAL(hasMath.FromJust(), true);

    // Property existance check.
    v8::Local<v8::Object> object = v8::Object::New(env.getIsolate());
    v8::Local<v8::String> fooKey = v8::String::NewFromUtf8(env.getIsolate(), "Foo", v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::Integer> fooValue = v8::Integer::New(env.getIsolate(), 33);
    v8::Maybe<bool> hasFoo = object->Has(env.getContext(), fooKey);

    ASSERT_EQUAL(hasFoo.FromJust(), false);

    // Define a Foo property.
    object->Set(env.getContext(), fooKey, fooValue);
    hasFoo = object->Has(env.getContext(), fooKey);
    v8::Local<v8::Value> fooGetValue = object->Get(fooKey);

    ASSERT_EQUAL(hasFoo.FromJust(), true);
    ASSERT_EQUAL(fooGetValue->IsInt32(), true);
    ASSERT_EQUAL(fooGetValue->Int32Value(), 33);

    // Delete the Foo property.
    object->Delete(env.getContext(), fooKey);
    hasFoo = object->Has(env.getContext(), fooKey);
    ASSERT_EQUAL(hasFoo.FromJust(), false);

    // Define property index.
    object->Set(env.getContext(), 0, fooValue);
    v8::Local<v8::Value> idxGetValue = object->Get(0);

    ASSERT_EQUAL(idxGetValue->IsInt32(), true);
    ASSERT_EQUAL(idxGetValue->Int32Value(), 33);

    // Define property with attributes.
    v8::PropertyAttribute attrs = (v8::PropertyAttribute)(v8::PropertyAttribute::ReadOnly | v8::PropertyAttribute::DontDelete);
    object->DefineOwnProperty(env.getContext(), fooKey, fooValue, attrs);

    v8::Maybe<bool> canDelete = object->Delete(env.getContext(), fooKey);
    ASSERT_EQUAL(canDelete.FromJust(), false);

    v8::Maybe<bool> canOverride = object->Set(env.getContext(), fooKey, fooValue);
    ASSERT_EQUAL(canOverride.FromJust(), false);

    // Check SetPrototype.
    hasMath = object->Has(env.getContext(), math);
    ASSERT_EQUAL(hasMath.FromJust(), false);

    object->SetPrototype(global);
    hasMath = object->Has(env.getContext(), math);
    ASSERT_EQUAL(hasMath.FromJust(),true);

    // Define private property.
    v8::Local<v8::Private> privateKey = v8::Private::New(env.getIsolate(), fooKey);
    ASSERT_EQUAL(object->HasPrivate(env.getContext(), privateKey).FromJust(), false);

    object->SetPrivate(env.getContext(), privateKey, fooValue);

    ASSERT_EQUAL(object->HasPrivate(env.getContext(), privateKey).FromJust(), true);
    ASSERT_EQUAL(object->GetPrivate(env.getContext(), privateKey).ToLocalChecked()->IsInt32(), true);
    ASSERT_EQUAL(object->GetPrivate(env.getContext(), privateKey).ToLocalChecked()->Int32Value(), 33);

    return 0;
}