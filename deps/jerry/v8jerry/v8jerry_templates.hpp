#ifndef V8JERRY_TEMPLATES_HPP
#define V8JERRY_TEMPLATES_HPP

#include <v8.h>
#include <jerryscript.h>

#include "v8jerry_value.hpp"

class JerryFunctionTemplate;

struct PropertyEntry {
    enum Type {
        Value,
        Accessor,
    };

    Type type;
    JerryValue* key;
    JerryValue* value_or_getter;
    JerryValue* setter;
    v8::PropertyAttribute attribute;

    ~PropertyEntry() {
        delete key;

        if (value_or_getter != NULL) {
            delete value_or_getter;
        }

        if (setter != NULL) {
            delete setter;
        }
    }
};

class JerryTemplate : public JerryHandle {
public:
    void Set(JerryValue* key, JerryValue* value,  v8::PropertyAttribute attribute) {
        m_properties.push_back(new PropertyEntry{PropertyEntry::Value, key, value, NULL, attribute});
    }

    void SetAccessorProperty(JerryValue* key,
                             JerryValue* getter,
                             JerryValue* setter,
                             v8::PropertyAttribute attribute) {
        m_properties.push_back(new PropertyEntry{PropertyEntry::Accessor, key, getter, setter, attribute});
    }

    void InstallProperties(const jerry_value_t target);

protected:
    JerryTemplate(JerryHandle::Type type)
        : JerryHandle(type)
        , m_properties(0)
    {
    }

    void ReleaseProperties(void) {
        for (PropertyEntry* prop : m_properties) {
            delete prop;
        }
    }

    std::vector<PropertyEntry*> m_properties;
};


/* Getter/setter call types & methods */
union AccessorEntryGetter {
    v8::AccessorGetterCallback stringed;
    v8::AccessorNameGetterCallback named;
};

union AccessorEntrySetter {
    v8::AccessorSetterCallback stringed;
    v8::AccessorNameSetterCallback named;
};

struct AccessorEntry {
    JerryValue* name;
    AccessorEntryGetter getter;
    AccessorEntrySetter setter;
    bool is_named; // true if  the Named variants should be used

    JerryValue* data;   // Can be NULL
    v8::AccessControl settings;
    v8::PropertyAttribute attribute;

    AccessorEntry(JerryValue* name,
                  v8::AccessorGetterCallback getter,
                  v8::AccessorSetterCallback setter,
                  JerryValue* data,
                  v8::AccessControl settings,
                  v8::PropertyAttribute attribute)
        : name(name)
        , is_named(false)
        , data(data)
        , settings(settings)
        , attribute(attribute)
    {
        this->getter.stringed = getter;
        this->setter.stringed = setter;
    }

    AccessorEntry(JerryValue* name,
                  v8::AccessorNameGetterCallback getter,
                  v8::AccessorNameSetterCallback setter,
                  JerryValue* data,
                  v8::AccessControl settings,
                  v8::PropertyAttribute attribute)
        : name(name)
        , is_named(true)
        , data(data)
        , settings(settings)
        , attribute(attribute)
    {
        this->getter.named = getter;
        this->setter.named = setter;
    }

    ~AccessorEntry() {
        delete name;
        if (data != NULL) {
            delete data;
        }
    }
};

struct JerryV8GetterSetterHandlerData {
    bool is_setter;
    bool is_named;
    union {
        AccessorEntryGetter getter;
        AccessorEntrySetter setter;
    } v8;
    jerry_value_t external;
    AccessorEntry* accessor;

    static jerry_object_native_info_t TypeInfo;
};

enum JerryV8ProxyHandlerType {
    UNKNOWN,
    GET,
    SET,
    DELETE,
    QUERY,
    ENUMERATE,
    DEFINE_PROPERTY,
    GET_OWN_PROPERTY_DESC,
};

struct JerryV8ProxyHandlerConfiguration {
    size_t ref_count;
    v8::GenericNamedPropertyGetterCallback genericGetter;
    v8::GenericNamedPropertySetterCallback genericSetter;
    v8::GenericNamedPropertyQueryCallback genericQuery;
    v8::GenericNamedPropertyDeleterCallback genericDeleter;
    v8::GenericNamedPropertyEnumeratorCallback genericEnumerator;
    v8::GenericNamedPropertyDefinerCallback genericDefiner;
    v8::GenericNamedPropertyDescriptorCallback genericDescriptor;
    v8::PropertyHandlerFlags genericFlags;
    jerry_value_t genericData;
};

struct JerryV8ProxyHandlerData {
    JerryV8ProxyHandlerType handler_type;
    JerryV8ProxyHandlerConfiguration* configuration;

    static jerry_object_native_info_t TypeInfo;
};

class JerryObjectTemplate : public JerryTemplate {
public:
    JerryObjectTemplate()
        : JerryTemplate(ObjectTemplate)
        , m_constructor(NULL)
        , m_accessors(0)
        , m_internal_field_count(0)
        , m_proxy_handler(NULL)
    {
    }

    ~JerryObjectTemplate(void) {
        ReleaseProperties();

        if (m_proxy_handler) {
            jerry_release_value(m_proxy_handler->genericData);

            if (--m_proxy_handler->ref_count == 0) {
                delete m_proxy_handler;
            }
        }
    }

    JerryFunctionTemplate* Constructor(void) { return m_constructor; }
    void SetConstructor(JerryFunctionTemplate* constructor) { m_constructor = constructor; }

    void SetInteralFieldCount(int count) { m_internal_field_count = count; }

    void SetAccessor(JerryValue* name,
                     v8::AccessorGetterCallback getter,
                     v8::AccessorSetterCallback setter,
                     JerryValue* data,
                     v8::AccessControl settings,
                     v8::PropertyAttribute attribute) {
        m_accessors.push_back(new AccessorEntry(name, getter, setter, data, settings, attribute));
    }

    void SetAccessor(JerryValue* name,
                     v8::AccessorNameGetterCallback getter,
                     v8::AccessorNameSetterCallback setter,
                     JerryValue* data,
                     v8::AccessControl settings,
                     v8::PropertyAttribute attribute) {
        m_accessors.push_back(new AccessorEntry(name, getter, setter, data, settings, attribute));
    }

    void SetAccessor(AccessorEntry* entry) {
        m_accessors.push_back(entry);
    }

    void ReleaseProperties(void) {
        JerryTemplate::ReleaseProperties();

        for (AccessorEntry* entry : m_accessors) {
            delete entry;
        }
    }

    void SetProxyHandler(const v8::NamedPropertyHandlerConfiguration& configuration);
    bool HasProxyHandler(void) { return m_proxy_handler != NULL; }

    void InstallProperties(const jerry_value_t target);
    JerryValue* Proxify(JerryValue* target_instance);

    static bool SetAccessor(const jerry_value_t target, AccessorEntry* entry);

private:
    // If m_constructor is null then the default prototype is used
    // it must be released "manually"
    JerryFunctionTemplate* m_constructor;
    std::vector<AccessorEntry*> m_accessors;
    int m_internal_field_count;
    JerryV8ProxyHandlerConfiguration* m_proxy_handler;
};

struct JerryV8FunctionHandlerData {
    uint32_t ref_count;
    JerryFunctionTemplate* function_template;
    v8::FunctionCallback v8callback;

    static jerry_object_native_info_t TypeInfo;
};

class JerryFunctionTemplate : public JerryTemplate {
public:
    JerryFunctionTemplate()
        : JerryTemplate(FunctionTemplate)
        , m_prototype_template(NULL)
        , m_instance_template(NULL)
        , m_proto_template(NULL)
        , m_prototype(jerry_create_null())
        , m_external(jerry_create_undefined())
        , m_function(NULL)
        , m_signature(NULL)
    {
    }

    ~JerryFunctionTemplate(void) {
        ReleaseProperties();
        jerry_release_value(m_prototype);
        jerry_release_value(m_external);

        delete m_function;
    }

    bool HasInstanceTemplate(void) const { return m_instance_template != NULL; }
    JerryFunctionTemplate* protoTemplate(void) { return m_proto_template; }
    void Inherit(JerryFunctionTemplate* parent) { m_proto_template = parent; }

    void SetCallback(v8::FunctionCallback callback) { m_callback = callback; }
    void SetExternalData(jerry_value_t value) { m_external = value; }
    void SetSignature(JerryHandle* handle) { m_signature = handle; }
    bool HasSignature(void) const { return m_signature != NULL; }
    bool IsValidSignature(const JerryHandle* handle) { return m_signature == handle; }
    const JerryHandle* Signature(void) const { return m_signature; }

    v8::FunctionCallback callback(void) const { return m_callback; }
    jerry_value_t external(void) const { return m_external; }

    JerryObjectTemplate* PrototypeTemplate(void);
    JerryObjectTemplate* InstanceTemplate(void);
    JerryValue* GetFunction(void);
    jerry_value_t GetPrototype(void);
    void SetFunctionHandlerData(jerry_value_t object);

private:
    JerryObjectTemplate* m_prototype_template;
    JerryObjectTemplate* m_instance_template;
    JerryFunctionTemplate* m_proto_template;
    v8::FunctionCallback m_callback;
    jerry_value_t m_prototype;
    jerry_value_t m_external;
    JerryValue* m_function;
    JerryHandle* m_signature;
};

#endif /* V8JERRY_TEMPLATES_HPP */
