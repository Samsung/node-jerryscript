#include "v8jerry_templates.hpp"

#include "v8jerry_isolate.hpp"
#include "v8jerry_callback.hpp"

void JerryTemplate::InstallProperties(const jerry_value_t target) {
    for (PropertyEntry* prop : m_properties) {
        jerry_property_descriptor_t desc;
        jerry_init_property_descriptor_fields(&desc);

        switch (prop->type) {
            case PropertyEntry::Value: {
                desc.is_value_defined = true;
                desc.value = jerry_acquire_value(prop->value_or_getter->value());

                desc.is_writable_defined = true;
                if (!(prop->attribute & v8::PropertyAttribute::ReadOnly)) {
                    desc.is_writable = true;
                }
                break;
            }
            case PropertyEntry::Accessor: {
                if (prop->value_or_getter != NULL) {
                    desc.is_get_defined = true;
                    desc.getter = jerry_acquire_value(prop->value_or_getter->value());
                }

                if (prop->setter != NULL) {
                    desc.is_set_defined = true;
                    desc.setter = jerry_acquire_value(prop->setter->value());
                }
                break;
            }
        }

        desc.is_configurable_defined = true;
        if (!(prop->attribute & v8::PropertyAttribute::DontDelete)) {
          desc.is_configurable = true;
        }

        desc.is_enumerable_defined = true;
        if (!(prop->attribute & v8::PropertyAttribute::DontEnum)) {
          desc.is_enumerable = true;
        }

        jerry_value_t result = jerry_define_own_property(target, prop->key->value(), &desc);
        jerry_free_property_descriptor_fields(&desc);

        /* TODO: check isOK? */
        // bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
        jerry_release_value(result);
    }
}

/* TODO: (elecro) remove after correct _callback include */
jerry_value_t JerryV8GetterSetterHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt);

static void JerryV8GetterSetterHandlerDataFree(void *data_p) {
    JerryV8GetterSetterHandlerData* data = reinterpret_cast<JerryV8GetterSetterHandlerData*>(data_p);

    if (data->is_setter == false && data->external) {
        jerry_release_value(data->external);
    }

    delete data;
}

/* static */
jerry_object_native_info_t JerryV8GetterSetterHandlerData::TypeInfo = {
    .free_cb = JerryV8GetterSetterHandlerDataFree,
};

/* static */
bool JerryObjectTemplate::SetAccessor(const jerry_value_t target, AccessorEntry* entry) {
    jerry_property_descriptor_t prop_desc;
    jerry_init_property_descriptor_fields (&prop_desc);

    // TODO: is there always a getter?
    prop_desc.is_get_defined = true;
    prop_desc.getter = jerry_create_external_function(JerryV8GetterSetterHandler);
    {
        JerryV8GetterSetterHandlerData* data = new JerryV8GetterSetterHandlerData();
        data->v8.getter = entry->getter;
        data->external = NULL; // TODO
        data->is_setter = false;
        data->accessor = entry;

        jerry_set_object_native_pointer(prop_desc.getter, data, &JerryV8GetterSetterHandlerData::TypeInfo);
    }

    prop_desc.is_set_defined = (entry->setter.stringed != NULL);
    if (prop_desc.is_set_defined) {
        prop_desc.setter = jerry_create_external_function(JerryV8GetterSetterHandler); // TODO: connect setter callback;

        JerryV8GetterSetterHandlerData* data = new JerryV8GetterSetterHandlerData();
        data->v8.setter = entry->setter;
        data->external = NULL; // TODO
        data->is_setter = true;
        data->accessor = entry;

        jerry_set_object_native_pointer(prop_desc.setter, data, &JerryV8GetterSetterHandlerData::TypeInfo);
    }
    prop_desc.is_configurable_defined = true;
    prop_desc.is_configurable = true;

    prop_desc.is_enumerable_defined = true;
    prop_desc.is_enumerable = true;

    // TODO: handle settings and attributes
    jerry_value_t define_result = jerry_define_own_property(target, entry->name->value(), &prop_desc);
    bool isOk = !jerry_value_is_error(define_result) && jerry_get_boolean_value(define_result);
    jerry_release_value(define_result);

    jerry_free_property_descriptor_fields(&prop_desc);

    return isOk;
}

void JerryObjectTemplate::InstallProperties(const jerry_value_t target) {
    JerryTemplate::InstallProperties(target);

    for (AccessorEntry* entry : m_accessors) {
        JerryObjectTemplate::SetAccessor(target, entry);
    }

    if (m_internal_field_count) {
        JerryValue::CreateInternalFields(target, m_internal_field_count);
    }
}

void JerryObjectTemplate::SetProxyHandler(const v8::NamedPropertyHandlerConfiguration& configuration)
{
    m_proxy_handler = new JerryV8ProxyHandlerConfiguration;

    m_proxy_handler->ref_count = 1;
    m_proxy_handler->genericGetter = configuration.getter;
    m_proxy_handler->genericSetter = configuration.setter;
    m_proxy_handler->genericQuery = configuration.query;
    m_proxy_handler->genericDeleter = configuration.deleter;
    m_proxy_handler->genericEnumerator = configuration.enumerator;
    m_proxy_handler->genericDefiner = configuration.definer;
    m_proxy_handler->genericDescriptor = configuration.descriptor;
    m_proxy_handler->genericFlags = configuration.flags;

    if (configuration.data.IsEmpty()) {
        m_proxy_handler->genericData = jerry_create_undefined();
    } else {
        m_proxy_handler->genericData = jerry_acquire_value(reinterpret_cast<JerryValue*>(*configuration.data)->value());
    }
}

static void JerryV8ProxyHandlerDataFree(void *data_p) {
    JerryV8ProxyHandlerData* data = reinterpret_cast<JerryV8ProxyHandlerData*>(data_p);

    if (--data->configuration->ref_count == 0) {
        delete data->configuration;
    }

    delete data;
}

jerry_object_native_info_t JerryV8ProxyHandlerData::TypeInfo = {
    .free_cb = JerryV8ProxyHandlerDataFree,
};

static jerry_value_t BuildProxyHandlerMethod(JerryV8ProxyHandlerConfiguration* proxy_handler,
                                             JerryV8ProxyHandlerType handler_type) {
    jerry_value_t func = jerry_create_external_function(JerryV8ProxyHandler);

    proxy_handler->ref_count++;

    JerryV8ProxyHandlerData* data = new JerryV8ProxyHandlerData();
    data->configuration = proxy_handler;
    data->handler_type = handler_type;

    /* Create weak reference. */
    jerry_value_t name = jerry_create_string((const jerry_char_t *)"0");
    jerry_set_internal_property(func, name, proxy_handler->genericData);
    jerry_release_value(name);

    jerry_set_object_native_pointer(func, data, &JerryV8ProxyHandlerData::TypeInfo);
    return func;
}

JerryValue* JerryObjectTemplate::Proxify(JerryValue* target_instance) {
    JerryValue* handler = JerryValue::NewObject();

    struct handlers {
        const char* name;
        JerryV8ProxyHandlerType type;
    } types[] = {
        { "get", GET },
        { "set", SET },
        { "deleteProperty", DELETE },
        { "has", QUERY },
        { "ownKeys", ENUMERATE },
        { "defineProperty", DEFINE_PROPERTY },
        { "getOwnPropertyDescriptor", GET_OWN_PROPERTY_DESC },
    };

    for (size_t idx = 0; idx < sizeof(types) / sizeof(types[0]); idx++) {
        JerryValue key(jerry_create_string((const jerry_char_t*) types[idx].name));
        JerryValue method(BuildProxyHandlerMethod(m_proxy_handler, types[idx].type));

        handler->SetProperty(&key, &method);
    }

    uint32_t options = JERRY_PROXY_SKIP_GET_CHECKS | JERRY_PROXY_SKIP_GET_OWN_PROPERTY_CHECKS;
    jerry_value_t proxy = jerry_create_special_proxy(target_instance->value(), handler->value(), options);

    delete target_instance;
    delete handler;

    return new JerryValue(proxy);
}


/* JerryFunctionTemplate */

JerryObjectTemplate* JerryFunctionTemplate::PrototypeTemplate(void) {
    if (!m_prototype_template) {
        m_prototype_template = new JerryObjectTemplate();
        JerryIsolate::GetCurrent()->AddTemplate(m_prototype_template);
    }

    return m_prototype_template;
}

JerryObjectTemplate* JerryFunctionTemplate::InstanceTemplate(void) {
    if (!m_instance_template) {
        m_instance_template = new JerryObjectTemplate();
        m_instance_template->SetConstructor(this);
        JerryIsolate::GetCurrent()->AddTemplate(m_instance_template);
    }

    return m_instance_template;
}

static void JerryV8FunctionHandlerDataFree(void* data) {
    JerryV8FunctionHandlerData* function_handler = reinterpret_cast<JerryV8FunctionHandlerData*>(data);

    if (--function_handler->ref_count == 0) {
        delete function_handler;
    }
}

jerry_object_native_info_t JerryV8FunctionHandlerData::TypeInfo = {
    .free_cb = JerryV8FunctionHandlerDataFree,
};

/* TODO: (elecro) remove after correct _callback include */
jerry_value_t JerryV8FunctionHandler(
    const jerry_value_t function_obj,
    const jerry_value_t this_val,
    const jerry_value_t args_p[],
    const jerry_length_t args_cnt);

JerryValue* JerryFunctionTemplate::GetFunction(void) {
    if (m_function != NULL) {
        return m_function;
    }

    // TODO: have function per context
    jerry_value_t jfunction = jerry_create_external_function(JerryV8FunctionHandler);

    JerryV8FunctionHandlerData* data = new JerryV8FunctionHandlerData();
    data->ref_count = 1;
    data->function_template = this; // Required so we can do instance checks.
    data->v8callback = m_callback;

    jerry_set_object_native_pointer(jfunction, data, &JerryV8FunctionHandlerData::TypeInfo);

    InstallProperties(jfunction);

    m_function = new JerryValue(jfunction);

    // Install the function prototype if there is any
    if (m_prototype_template != NULL) {
        m_prototype = jerry_create_object();
        m_prototype_template->InstallProperties(m_prototype);

        JerryValue proto_string(jerry_create_string((const jerry_char_t*)"prototype"));
        jerry_release_value (jerry_set_property(m_function->value(), proto_string.value(), m_prototype));

        jerry_property_descriptor_t desc;
        jerry_init_property_descriptor_fields(&desc);
        desc.is_value_defined = true;
        desc.is_writable_defined = true;
        desc.is_writable = true;
        desc.is_enumerable_defined = true;
        desc.is_enumerable = false;
        desc.is_configurable_defined = true;
        desc.is_configurable = true;
        desc.value = jerry_acquire_value (jfunction);

        JerryValue constructor_string(jerry_create_string((const jerry_char_t*)"constructor"));
        jerry_release_value (jerry_define_own_property(m_prototype, constructor_string.value(), &desc));
        jerry_free_property_descriptor_fields(&desc);

        if (m_proto_template != NULL)
        {
            jerry_set_prototype (m_prototype, m_proto_template->GetPrototype());
        }
    }

    return m_function;
}

jerry_value_t JerryFunctionTemplate::GetPrototype(void) {
    if (m_function != NULL) {
        return m_prototype;
    }

    GetFunction();
    return m_prototype;
}

void JerryFunctionTemplate::SetFunctionHandlerData(jerry_value_t object) {
    if (m_function == NULL) {
        GetFunction();
    }

    void *native_p = NULL;
    jerry_get_object_native_pointer(m_function->value(), &native_p, &JerryV8FunctionHandlerData::TypeInfo);

    ((JerryV8FunctionHandlerData*)native_p)->ref_count++;
    jerry_set_object_native_pointer(object, native_p, &JerryV8FunctionHandlerData::TypeInfo);
}
