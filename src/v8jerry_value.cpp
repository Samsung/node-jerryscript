#include "v8jerry_value.hpp"

#include "v8jerry_isolate.hpp"

static void JerryV8InternalFieldDataFree(void *data) {
    delete reinterpret_cast<JerryV8InternalFieldData*>(data);
}

static jerry_object_native_info_t JerryV8InternalFieldTypeInfo = {
    .free_cb = JerryV8InternalFieldDataFree,
};

static jerry_object_native_info_t JerryV8ExternalTypeInfo = {
    .free_cb = NULL,
};

bool JerryValue::SetProperty(JerryValue* key, JerryValue* value) {
    // TODO: NULL check assert for key, value
    jerry_value_t result = jerry_set_property(m_value, key->value(), value->value());
    bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isOk;
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

JerryValue* JerryValue::GetPropertyIdx(uint32_t idx) {
    jerry_value_t prop = jerry_get_property_by_index(m_value, idx);
    return JerryValue::TryCreateValue(JerryIsolate::GetCurrent(), prop);
}

static const jerry_object_native_info_t JerryV8ObjectContextTypeInfo = {
    /* native_pointer stores JerryContext* */
    .free_cb = NULL
};

JerryContext* JerryValue::GetObjectCreationContext(void) {
    void* data_p;
    bool has_p = jerry_get_object_native_pointer(m_value, &data_p, &JerryV8ObjectContextTypeInfo);
    if (!has_p) {
        return NULL;
    }

    return reinterpret_cast<JerryContext*>(data_p);
}

/* static */
JerryValue* JerryValue::NewObject(void) {
    jerry_value_t object = jerry_create_object();

    JerryContext* ctx = JerryIsolate::GetCurrent()->CurrentContext();

    jerry_set_object_native_pointer(object, ctx, &JerryV8ObjectContextTypeInfo);

    return new JerryValue(object);
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

    this->SetProperty(&name, value);
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
