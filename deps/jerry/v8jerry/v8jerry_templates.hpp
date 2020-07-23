#ifndef V8JERRY_TEMPLATES_HPP
#define V8JERRY_TEMPLATES_HPP

#include <v8.h>
#include <jerryscript.h>

#include "v8jerry_value.hpp"

class JerryFunctionTemplate;

struct PropertyEntry {
    enum Type {
        Value,
        GetterSetter,
    };

    Type type;
    JerryValue* key;
    JerryValue* value;
    JerryValue* setter;
    v8::PropertyAttribute attribute;


    ~PropertyEntry() {
        delete key;

        if (value != NULL) {
            delete value;
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
        m_properties.push_back(new PropertyEntry{PropertyEntry::GetterSetter, key, getter, setter, attribute});
    }


    const std::vector<PropertyEntry*>& properties(void) const { return m_properties; }

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

class JerryObjectTemplate : public JerryTemplate {
public:
    JerryObjectTemplate()
        : JerryTemplate(ObjectTemplate)
        , m_function_template(NULL)
        , m_accessors(0)
        , m_internal_field_count(0)
    {
    }

    ~JerryObjectTemplate(void) {
        ReleaseProperties();
    }

    void SetInteralFieldCount(int count) { m_internal_field_count = count; }
    void SetConstructor(JerryFunctionTemplate* function_template) { m_function_template = function_template; }

    JerryFunctionTemplate* FunctionTemplate(void) { return m_function_template; }

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

    static bool SetAccessor(const jerry_value_t target, AccessorEntry* entry);
    void InstallProperties(const jerry_value_t target);

    void ReleaseProperties(void) {
        JerryTemplate::ReleaseProperties();

        for (AccessorEntry* entry : m_accessors) {
            delete entry;
        }
    }

private:
    // If the m_function_template is null then this is not bound to a function
    // it must be released "manually"
    JerryFunctionTemplate* m_function_template;
    std::vector<AccessorEntry*> m_accessors;
    int m_internal_field_count;
};

/* Simple function call types & methods */
struct JerryV8FunctionHandlerData {
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
        , m_external(jerry_create_undefined())
        , m_function(NULL)
        , m_signature(NULL)
    {
    }

    ~JerryFunctionTemplate(void) {
        ReleaseProperties();
        jerry_release_value(m_external);

        delete m_function;
    }

    JerryObjectTemplate* PrototypeTemplate(void);
    void Inherit(JerryObjectTemplate* parent);
    JerryObjectTemplate* InstanceTemplate(void);

    bool HasInstanceTemplate(void) const { return m_instance_template != NULL; }

    void SetCallback(v8::FunctionCallback callback) { m_callback = callback; }
    void SetExternalData(jerry_value_t value) { m_external = value; }
    void SetSignature(JerryHandle* handle) { m_signature = handle; }
    bool HasSignature(void) const { return m_signature != NULL; }
    bool IsValidSignature(const JerryHandle* handle) { return m_signature == handle; }
    const JerryHandle* Signature(void) const { return m_signature; }

    v8::FunctionCallback callback(void) const { return m_callback; }
    jerry_value_t external(void) const { return m_external; }

    JerryValue* GetFunction(void);
private:
    JerryObjectTemplate* m_prototype_template;
    JerryObjectTemplate* m_instance_template;
    v8::FunctionCallback m_callback;
    jerry_value_t m_external;
    JerryValue* m_function;
    JerryHandle* m_signature;
};

#endif /* V8JERRY_TEMPLATES_HPP */

