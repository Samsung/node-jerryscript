#include "v8jerry_value.hpp"

#include "v8jerry_isolate.hpp"
#include "assert.h"

static void JerryV8InternalFieldDataFree(void* native_p, jerry_object_native_info_t* info_p) {
    (void) info_p;

    delete reinterpret_cast<JerryV8InternalFieldData*>(native_p);
}

static jerry_object_native_info_t JerryV8InternalFieldTypeInfo = {
    .free_cb = JerryV8InternalFieldDataFree,
    .number_of_references = 0,
    .offset_of_references = 0,
};

static jerry_object_native_info_t JerryV8ExternalTypeInfo = {
    .free_cb = NULL,
};

static void JerryV8BackingStoreCallback(void* native_p, jerry_object_native_info_t* info_p) {
    (void) info_p;

    if (native_p != NULL) {
        delete static_cast<JerryBackingStore*>(native_p);
    }
};

static jerry_object_native_info_t JerryV8BackingStoreInfo = {
    .free_cb = JerryV8BackingStoreCallback,
    .number_of_references = 0,
    .offset_of_references = 0,
};

bool JerryValue::SetProperty(JerryValue* key, JerryValue* value) {
    // TODO: NULL check assert for key, value
    jerry_value_t result = jerry_set_property(m_value, key->value(), value->value());
    bool isOk = !jerry_value_is_error(result) && jerry_value_is_true(result);
    jerry_release_value(result);

    return isOk;
}

bool JerryValue::SetPrivateProperty(JerryValue* key, JerryValue* value) {
    return jerry_set_internal_property (m_value, key->value(), value->value());
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

JerryValue* JerryValue::GetOwnPropertyDescriptor(const JerryValue& jkey) const {
    jerry_value_t key = jkey.value();
    jerry_value_t descriptor = JerryIsolate::GetCurrent()->HelperGetOwnPropDesc().Call(m_value, &key, 1);

    return new JerryValue(descriptor);
}

JerryValue* JerryValue::GetOwnPropertyNames() const {
    return new JerryValue(JerryIsolate::GetCurrent()->HelperGetOwnPropNames().Call(m_value, NULL, 0));
}

jerry_value_t JerryValue::GetObjectCreationContext(void) {
    jerry_value_t name = jerry_create_string((const jerry_char_t*)"_CC_");
    jerry_value_t result = jerry_get_internal_property(value(), name);
    jerry_release_value(name);
    return result;
}

/* static */
JerryValue* JerryValue::NewPromise(void) {
    jerry_value_t promise = jerry_create_promise();

    JerryValue* ctx = JerryIsolate::GetCurrent()->CurrentContext();

    jerry_value_t name = jerry_create_string((const jerry_char_t*)"_CC_");
    jerry_set_internal_property(promise, name, ctx->value());
    jerry_release_value(name);

    return new JerryValue(promise);
}

/* static */
JerryValue* JerryValue::NewObject(void) {
    jerry_value_t object = jerry_create_object();

    JerryValue* ctx = JerryIsolate::GetCurrent()->CurrentContext();

    jerry_value_t name = jerry_create_string((const jerry_char_t*)"_CC_");
    jerry_set_internal_property(object, name, ctx->value());
    jerry_release_value(name);

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

static void JerryV8ContextDataFree(void* native_p, jerry_object_native_info_t* info_p) {
    (void) info_p;

    delete reinterpret_cast<JerryV8ContextData*>(native_p);
}

static jerry_object_native_info_t JerryV8ContextTypeInfo = {
    .free_cb = JerryV8ContextDataFree,
    .number_of_references = 0,
    .offset_of_references = 0,
};

/* static */
JerryValue* JerryValue::NewContextObject(JerryIsolate* iso) {
    jerry_value_t object = jerry_create_realm();

    jerry_value_t old_realm = jerry_set_realm(object);
    InjectGlobalFunctions();
    jerry_set_realm(old_realm);

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

    ctx->isolate->PopContext();
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

    data->fields[idx] = reinterpret_cast<void*>(value->value());

    /* Create reference to the property */
    char buffer[16];
    char* buffer_ptr = buffer + sizeof(buffer) - 1;

    *buffer_ptr = '\0';

    do {
        *(--buffer_ptr) = (idx % 10) + '0';
        idx /= 10;
    } while (idx > 0);

    JerryValue name(jerry_create_string_from_utf8((const jerry_char_t *)buffer_ptr));
    SetInternalProperty(&name, value);
}

void JerryValue::SetInternalField(int idx, void* value) {
    JerryV8InternalFieldData* data = GetInternalFieldData(idx);

    if (data == NULL) {
        fprintf(stderr, "ERROR!\n");
        abort();
        return;
    }

    data->fields[idx] = value;
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
JerryValue* JerryValue::TryCreateValue(v8::Isolate* isolate, jerry_value_t value) {
    if (V8_UNLIKELY(jerry_value_is_error(value))) {
        JerryIsolate::fromV8(isolate)->SetError(value);
        return NULL;
    }
    return new JerryValue(value);
}

static void JerryV8WeakCallback(void* native_p, jerry_object_native_info_t* info_p) {
    (void) info_p;

    JerryV8WeakReference* weak_ref = reinterpret_cast<JerryV8WeakReference*>(native_p);

    while (weak_ref != NULL) {
        void* parameter = parameter = weak_ref->data;
        void* tmp_fields[v8::kEmbedderFieldsInWeakCallback];
        void** tmp_fields_ptr = reinterpret_cast<void**>(&tmp_fields);
        void** embedder_fields = tmp_fields_ptr;

        weak_ref->persistent->m_type = JerryHandle::PersistentDeletedValue;

        v8::WeakCallbackInfo<void> info(v8::Isolate::GetCurrent(), parameter, embedder_fields, &weak_ref->callback);
        weak_ref->callback(info);

        JerryV8WeakReference* next_weak_ref = weak_ref->next;

        delete weak_ref;
        weak_ref = next_weak_ref;
    }
}

static jerry_object_native_info_t JerryV8WeakReferenceInfo = {
    .free_cb = JerryV8WeakCallback,
    .number_of_references = 0,
    .offset_of_references = 0,
};

void JerryValue::MakeWeak(v8::WeakCallbackInfo<void>::Callback weak_callback, v8::WeakCallbackType type, void* data) {
    m_type = PersistentWeakValue;

    void* weak_ref_head = NULL;
    jerry_get_object_native_pointer(value(), &weak_ref_head, &JerryV8WeakReferenceInfo);

    JerryV8WeakReference* weak_ref = new JerryV8WeakReference(reinterpret_cast<JerryV8WeakReference*>(weak_ref_head), this, weak_callback, type, data);
    jerry_set_object_native_pointer(value(), reinterpret_cast<void*>(weak_ref), &JerryV8WeakReferenceInfo);
    jerry_release_value(value());
}

void* JerryValue::ClearWeak() {
    m_type = PersistentValue;

    void* data = NULL;
    jerry_get_object_native_pointer(value(), &data, &JerryV8WeakReferenceInfo);

    JerryV8WeakReference* weak_ref = reinterpret_cast<JerryV8WeakReference*>(data);

    if (weak_ref->persistent == this)
    {
        jerry_set_object_native_pointer (value(), reinterpret_cast<void*>(weak_ref->next), &JerryV8WeakReferenceInfo);
    }
    else
    {
        JerryV8WeakReference* prev_weak_ref = weak_ref;
        weak_ref = weak_ref->next;

        while (true) {
            if (weak_ref->persistent == this) {
                prev_weak_ref->next = weak_ref->next;
                break;
            }

            prev_weak_ref = weak_ref;
            weak_ref = weak_ref->next;
        }
    }

    data = weak_ref->data;
    delete weak_ref;

    return data;
}

JerryValue* JerryValue::ToInteger(v8::Isolate* isolate) const {
    jerry_value_t number = jerry_value_to_number(m_value);

    if (V8_UNLIKELY(jerry_value_is_error(number))) {
        JerryIsolate::fromV8(isolate)->SetError(number);
        return NULL;
    }

    double integer = jerry_value_as_integer(number);
    jerry_release_value(number);
    return new JerryValue(jerry_create_number(integer));
}
