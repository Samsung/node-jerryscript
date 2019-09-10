#include "v8env.h"
#include "assert.h"

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    v8::Local<v8::ObjectTemplate> tmplt = v8::ObjectTemplate::New(env.getIsolate());
    tmplt->SetInternalFieldCount(2);

    v8::Local<v8::Object> obj1 = tmplt->NewInstance(env.getContext()).ToLocalChecked();

    obj1->SetInternalField(0, v8::Object::New(env.getIsolate()));

    ASSERT_EQUAL(obj1->InternalFieldCount(), 2);

    void* in_ptr = (void*)0x11223344;
    obj1->SetAlignedPointerInInternalField(1, in_ptr);

    void* ptr = obj1->GetAlignedPointerFromInternalField(1);

    ASSERT_EQUAL(ptr, in_ptr);

    v8::Local<v8::Value> obj_internal = obj1->GetInternalField(0);

    ASSERT_EQUAL(obj_internal->IsObject(), true);

    return 0;
}
