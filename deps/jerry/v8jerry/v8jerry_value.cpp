#include "v8jerry_value.hpp"

#include "v8jerry_isolate.hpp"
#include "assert.h"

static void JerryV8InternalFieldDataFree(void *data) {
    delete reinterpret_cast<JerryV8InternalFieldData*>(data);
}

static jerry_object_native_info_t JerryV8InternalFieldTypeInfo = {
    .free_cb = JerryV8InternalFieldDataFree,
};

static jerry_object_native_info_t JerryV8ExternalTypeInfo = {
    .free_cb = NULL,
};

static void JerryV8WeakCallback(void* data) {
    if (data == NULL) {
        return;
    }

    JerryV8WeakReferenceData* weak_cb_data = static_cast<JerryV8WeakReferenceData*>(data);

    void* parameter = NULL;
    void** embedder_fields = NULL;

    if (weak_cb_data->type == v8::WeakCallbackType::kInternalFields) {
        embedder_fields = reinterpret_cast<void**>(weak_cb_data->data);
    } else {
        parameter = weak_cb_data->data;
        embedder_fields = new void*[v8::kEmbedderFieldsInWeakCallback];
    }

    v8::WeakCallbackInfo<void> info(v8::Isolate::GetCurrent(), parameter, embedder_fields, &weak_cb_data->callback);
    weak_cb_data->callback(info);

    delete [] embedder_fields;
    delete weak_cb_data;
};

static jerry_object_native_info_t JerryV8WeakReferenceInfo = {
    .free_cb = JerryV8WeakCallback,
};

static void JerryV8BackingStoreCallback(void* data) {
    if (data != NULL) {
        delete static_cast<JerryBackingStore*>(data);
    }
};

static jerry_object_native_info_t JerryV8BackingStoreInfo = {
    .free_cb = JerryV8BackingStoreCallback,
};

bool JerryValue::SetProperty(JerryValue* key, JerryValue* value) {
    // TODO: NULL check assert for key, value
    jerry_value_t result = jerry_set_property(m_value, key->value(), value->value());
    bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isOk;
}

bool JerryValue::SetPrivateProperty(JerryValue* key, JerryValue* value) {
    return jerry_set_internal_property (m_value, key->value(), value->value());
}

bool JerryValue::SetPropertyIdx(uint32_t idx, JerryValue* value) {
    // TODO: NULL check assert for value
    jerry_value_t result = jerry_set_property_by_index(m_value, idx, value->value());
    bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isOk;
}

JerryValue* JerryValue::GetProperty(JerryValue* key) {
    jerry_value_t prop = jerry_get_property(m_value, key->value());
    return JerryValue::TryCreateValue(JerryIsolate::GetCurrent(), prop);
}

JerryValue* JerryValue::GetPrivateProperty(JerryValue* key) {
    jerry_value_t prop = jerry_get_internal_property(m_value, key->value());
    return JerryValue::TryCreateValue(JerryIsolate::GetCurrent(), prop);
}

JerryValue* JerryValue::GetPropertyIdx(uint32_t idx) {
    jerry_value_t prop = jerry_get_property_by_index(m_value, idx);
    return JerryValue::TryCreateValue(JerryIsolate::GetCurrent(), prop);
}

bool JerryValue::SetInternalProperty(JerryValue* key, JerryValue* value) {
    return jerry_set_internal_property(m_value, key->value(), value->value());
}

bool JerryValue::HasInternalProperty(JerryValue* key) {
    return jerry_has_internal_property(m_value, key->value());
}

bool JerryValue::DeleteInternalProperty(JerryValue* key) {
    return jerry_delete_internal_property(m_value, key->value());
}

JerryValue* JerryValue::GetInternalProperty(JerryValue* key) {
    jerry_value_t prop = jerry_get_internal_property(m_value, key->value());
    return JerryValue::TryCreateValue(JerryIsolate::GetCurrent(), prop);
}

JerryValue* JerryValue::GetOwnPropertyDescriptor(const JerryValue& jkey) const {
    jerry_value_t key = jkey.value();
    jerry_value_t descriptor = JerryIsolate::GetCurrent()->HelperGetOwnPropDesc().Call(m_value, &key, 1);

    return new JerryValue(descriptor);
}

JerryValue* JerryValue::GetOwnPropertyNames() const {
    return new JerryValue(JerryIsolate::GetCurrent()->HelperGetOwnPropNames().Call(m_value, NULL, 0));
}

JerryValue* JerryValue::GetPropertyNames() const {
    return new JerryValue(JerryIsolate::GetCurrent()->HelperGetPropNames().Call(m_value, NULL, 0));
}

static const jerry_object_native_info_t JerryV8ObjectContextTypeInfo = {
    /* native_pointer stores JerryContext (aka JerryValue*) which will be freed via the handlescope */
    .free_cb = NULL
};

JerryValue* JerryValue::GetObjectCreationContext(void) {
    void* data_p;
    bool has_p = jerry_get_object_native_pointer(m_value, &data_p, &JerryV8ObjectContextTypeInfo);
    if (!has_p) {
        return NULL;
    }

    return reinterpret_cast<JerryValue*>(data_p);
}

/* static */
JerryValue* JerryValue::NewPromise(void) {
    jerry_value_t promise = jerry_create_promise();

    JerryValue* ctx = JerryIsolate::GetCurrent()->CurrentContext();

    jerry_set_object_native_pointer(promise, ctx, &JerryV8ObjectContextTypeInfo);

    return new JerryValue(promise);
}

/* static */
JerryValue* JerryValue::NewObject(void) {
    jerry_value_t object = jerry_create_object();

    JerryValue* ctx = JerryIsolate::GetCurrent()->CurrentContext();

    jerry_set_object_native_pointer(object, ctx, &JerryV8ObjectContextTypeInfo);

    return new JerryValue(object);
}

/* static */
JerryValue* JerryValue::NewArrayBuffer(JerryBackingStore *backingStore) {
    jerry_value_t buffer = jerry_create_arraybuffer_external(backingStore->byteLength(), (uint8_t*)backingStore->data(), NULL);

    jerry_set_object_native_pointer(buffer, backingStore, &JerryV8BackingStoreInfo);

    return new JerryValue(buffer);
}

/* static */
JerryValue* JerryValue::NewTypedArray(JerryValue* array_buffer,
                                      size_t byte_offset, size_t length, jerry_typedarray_type_t type) {
    return new JerryValue(jerry_create_typedarray_for_arraybuffer_sz(type, array_buffer->value(), (jerry_length_t) byte_offset, (jerry_length_t) length));
}

JerryBackingStore* JerryValue::GetBackingStore(void) const {
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8BackingStoreInfo);
    if (!has_data) {
        // FOR JS created Arraybuffers
        JerryBackingStore *backingStore = new JerryBackingStore (jerry_get_arraybuffer_pointer(m_value), jerry_get_arraybuffer_byte_length(m_value));
        jerry_set_object_native_pointer(m_value, backingStore, &JerryV8BackingStoreInfo);
        return backingStore;
    }

    return reinterpret_cast<JerryBackingStore*>(native_p);
}

/* static */
JerryValue* JerryValue::NewExternal(void* ptr) {
    JerryValue* jobject = JerryValue::NewObject();
    jerry_set_object_native_pointer(jobject->value(), ptr, &JerryV8ExternalTypeInfo);

    JerryIsolate* iso = JerryIsolate::GetCurrent();

    jerry_release_value(iso->HelperConversionFailer().Call(jobject->value()));
    return jobject;
}

void* JerryValue::GetExternalData(void) const {
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8ExternalTypeInfo);
    if (!has_data) {
        return NULL;
    }

    return native_p;
}

bool JerryValue::IsExternal() const {
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8ExternalTypeInfo);
    return has_data;
}

struct JerryV8ContextData {
    JerryIsolate* isolate;
    std::vector<void*> embedderData;
};

static void JerryV8ContextDataFree(void *data) {
    delete reinterpret_cast<JerryV8ContextData*>(data);
}

static jerry_object_native_info_t JerryV8ContextTypeInfo = {
    .free_cb = JerryV8ContextDataFree,
};

/* static */
JerryValue* JerryValue::NewContextObject(JerryIsolate* iso) {
    jerry_value_t object = jerry_create_object();

    JerryV8ContextData* ctx_data = new JerryV8ContextData{iso, {}};

    jerry_set_object_native_pointer(object, ctx_data, &JerryV8ContextTypeInfo);
    // NOTE: NODE_BINDING_LIST_INDEX 36
    CreateInternalFields(object, 37);
    return new JerryValue(object);
}

bool JerryValue::IsContextObject(void) {
    return jerry_get_object_native_pointer(m_value, NULL, &JerryV8ContextTypeInfo);
}

JerryV8ContextData* JerryValue::ContextGetData(void) {
    JerryV8ContextData* data;
    bool has_native = jerry_get_object_native_pointer(m_value, (void**)&data, &JerryV8ContextTypeInfo);
    if (!has_native) {
        return NULL;
    }

    return data;
}

JerryIsolate* JerryValue::ContextGetIsolate(void) {
    assert(IsContextObject());

    JerryV8ContextData* ctx = ContextGetData();
    return ctx->isolate;
}

void JerryValue::ContextEnter(void) {
    assert(IsContextObject());

    JerryV8ContextData* ctx = ContextGetData();

    ctx->isolate->Enter();
    ctx->isolate->PushContext(this);
}

void JerryValue::ContextExit(void) {
    assert(IsContextObject());

    JerryV8ContextData* ctx = ContextGetData();

    ctx->isolate->PopContext(this);
    ctx->isolate->Exit();
}

void JerryValue::ContextSetEmbedderData(int index, void* value) {
    assert(IsContextObject());

    JerryV8ContextData* ctx = ContextGetData();

    if ((int)ctx->embedderData.size() <= index) {
        ctx->embedderData.resize(index + 1);
    }
    ctx->embedderData[index] = value;
}

void* JerryValue::ContextGetEmbedderData(int index) {
    assert(IsContextObject());

    JerryV8ContextData* ctx = ContextGetData();

    if (V8_UNLIKELY((int)ctx->embedderData.size() < index)) {
        return NULL;
    }

    return ctx->embedderData[index];
}


/* static */
void JerryValue::CreateInternalFields(jerry_value_t target, int field_count) {
    JerryV8InternalFieldData *data = new JerryV8InternalFieldData(field_count);
    jerry_set_object_native_pointer(target, data, &JerryV8InternalFieldTypeInfo);
}

JerryV8InternalFieldData* JerryValue::GetInternalFieldData(int idx) {
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8InternalFieldTypeInfo);
    if (!has_data) {
        return NULL;
    }

    JerryV8InternalFieldData* data = reinterpret_cast<JerryV8InternalFieldData*>(native_p);
    if (data->count <= idx) {
        fprintf(stderr, "ERROR! incorrect index (available slots: %d, tried with: %d\n", data->count, idx);
        abort();
        return  NULL;
    }

    return data;
}

void JerryValue::SetInternalField(int idx, JerryValue* value) {
    JerryV8InternalFieldData* data = GetInternalFieldData(idx);
    if (data == NULL) {
        fprintf(stderr, "ERROR!\n");
        abort();
        return;
    }

    data->fields[idx] = NULL;
    data->is_value[idx] = true;

    std::string internal_name = "$$internal_" + idx;
    JerryValue name(jerry_create_string_from_utf8((const jerry_char_t*)internal_name.c_str()));
    SetInternalProperty(&name, value);
}

void JerryValue::SetInternalField(int idx, void* value) {
    JerryV8InternalFieldData* data = GetInternalFieldData(idx);

    // TODO: maybe delete "$$internal_<x>" properties?
    data->fields[idx] = value;
    data->is_value[idx] = false;
}

int JerryValue::InternalFieldCount(void) {
    JerryV8InternalFieldData *data;
    bool has_data = jerry_get_object_native_pointer(m_value, reinterpret_cast<void**>(&data), &JerryV8InternalFieldTypeInfo);
    if (!has_data) {
        return 0;
    }

    return data->count;
}

/* static */
JerryValue* JerryValue::TryCreateValue(JerryIsolate* iso, jerry_value_t value) {
    if (V8_UNLIKELY(jerry_value_is_error(value))) {
        iso->SetError(value);

        return NULL;
    } else {
        return new JerryValue(value);
    }
}

bool JerryValue::IsWeakReferenced() {
    void* data_p = NULL;
    return jerry_get_object_native_pointer(m_value, &data_p, &JerryV8WeakReferenceInfo) && data_p != NULL;
}

void JerryValue::MakeWeak(v8::WeakCallbackInfo<void>::Callback weak_callback, v8::WeakCallbackType type, void* data) {
    assert(IsWeakReferenced() == false);

    JerryV8WeakReferenceData* weak_data = new JerryV8WeakReferenceData(weak_callback, type, data);
    jerry_set_object_native_pointer(value(), weak_data, &JerryV8WeakReferenceInfo);

    JerryIsolate::GetCurrent()->AddAsWeak(this);
}

void* JerryValue::ClearWeak() {
    if(!IsWeakReferenced()) {
        return NULL;
    }

    JerryV8WeakReferenceData* weak_data;
    jerry_get_object_native_pointer (m_value, reinterpret_cast<void**>(&weak_data), &JerryV8WeakReferenceInfo);
    jerry_delete_object_native_pointer(m_value, &JerryV8WeakReferenceInfo);

    JerryIsolate::GetCurrent()->RemoveAsWeak(this);

    return weak_data->data;
}

void JerryValue::RunWeakCleanup() {
    assert(IsWeakReferenced() == true);

    JerryV8WeakReferenceData* weak_data;
    jerry_get_object_native_pointer (m_value, reinterpret_cast<void**>(&weak_data), &JerryV8WeakReferenceInfo);
    jerry_set_object_native_pointer (m_value, NULL, &JerryV8WeakReferenceInfo);
    //jerry_delete_object_native_pointer(m_value, &JerryV8WeakReferenceInfo);

    JerryV8WeakReferenceInfo.free_cb(weak_data);
}

jerry_value_t JerryString::FromBuffer (const char* buffer, int length) {
    if (length == -1) {
        length = strlen(buffer);
    }

    return jerry_create_string_sz_from_utf8((const jerry_char_t*)buffer, length);
}

namespace v8 {
    namespace internal {
        class Heap {
        public:
            static void DisposeExternalString(v8::String::ExternalStringResourceBase* external_string) {
                external_string->Dispose();
            }
        };
    }
}

JerryExternalString::~JerryExternalString() {
    v8::internal::Heap::DisposeExternalString(m_resource);
}
