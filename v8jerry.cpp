#include <v8.h>
#include <v8-debug.h>
#include <v8-profiler.h>
#include <libplatform/libplatform.h>

#include <cstring>
#include <stack>
#include <vector>
#include <algorithm>

#include "jerryscript.h"
#include "jerryscript-port-default.h"

/* Tooling helper methods*/

static jerry_value_t BuildHelperMethod(const char* fn_args, const char* fn_body) {
    jerry_value_t method = jerry_parse_function(NULL, 0,
                                 reinterpret_cast<const jerry_char_t*>(fn_args), strlen(fn_args),
                                 reinterpret_cast<const jerry_char_t*>(fn_body), strlen(fn_body),
                                 JERRY_PARSE_NO_OPTS);
    if (jerry_value_is_error(method)) {
        fprintf(stderr, "Failed to build helper method initialize at: %s:%d\nfunction (%s) {\n%s\n}", __FILE__, __LINE__, fn_args, fn_body);
        abort();
    }
    return method;
}

/* Jerry <-> V8 binding classes */
class JerryContext;
class JerryFunctionTemplate;
class JerryHandleScope;
class JerryIsolate;
class JerryObjectTemplate;
class JerryValue;

class JerryHandle {
public:
    enum Type {
        Context,
        FunctionTemplate,
        ObjectTemplate,
        Value,
    };

    JerryHandle() {}

    JerryHandle(Type type)
        : m_type(type)
    {}

    Type type() const { return m_type; }

private:
    Type m_type;
};

template class std::vector<JerryHandle*>;

struct JerryV8InternalFieldData {
    // TODO: maybe use raw pointers to reduce memory?
    int count;
    std::vector<void*> fields;
    std::vector<bool> is_value;

    JerryV8InternalFieldData(int count)
        : count(count)
        , fields(count, NULL)
        , is_value(count, false)
    {
    }
};

void JerryV8InternalFieldDataFree(void *data) {
    delete reinterpret_cast<JerryV8InternalFieldData*>(data);
}

static jerry_object_native_info_t JerryV8InternalFieldTypeInfo = {
    .free_cb = JerryV8InternalFieldDataFree,
};

static jerry_object_native_info_t JerryV8ExternalTypeInfo = {
    .free_cb = NULL,
};

class JerryValue : public JerryHandle {
public:
    JerryValue()
        : JerryHandle(JerryHandle::Value)
    {}

    JerryValue(jerry_value_t value)
        : JerryHandle(JerryHandle::Value)
        , m_value(value)
    {}

    ~JerryValue(void) {
        if (m_value) {
            jerry_release_value(m_value);
        }
    }

    //jerry_value_t value() { return m_value; }
    jerry_value_t value() const { return m_value; }

    bool SetProperty(JerryValue* key, JerryValue* value) {
        // TODO: NULL check assert for key, value
        jerry_value_t result = jerry_set_property(m_value, key->value(), value->value());
        bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
        jerry_release_value(result);

        return isOk;
    }

    bool SetPropertyIdx(uint32_t idx, JerryValue* value) {
        // TODO: NULL check assert for value
        jerry_value_t result = jerry_set_property_by_index(m_value, idx, value->value());
        bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
        jerry_release_value(result);

        return isOk;
    }

    JerryValue* GetProperty(JerryValue* key) {
        return new JerryValue(jerry_get_property(m_value, key->value()));
    }

    JerryValue* GetPropertyIdx(uint32_t idx) {
        return new JerryValue(jerry_get_property_by_index(m_value, idx));
    }

    bool IsString() const { return jerry_value_is_string(m_value); }
    bool IsBoolean() const { return jerry_value_is_boolean(m_value); }
    bool IsFalse() const { return jerry_value_is_boolean(m_value) && !jerry_get_boolean_value(m_value); }
    bool IsTrue() const { return jerry_value_is_boolean(m_value) && jerry_get_boolean_value(m_value); }
    bool IsPromise() const { return jerry_value_is_promise(m_value); }
    bool IsArray() const { return jerry_value_is_array(m_value); }
    bool IsNumber() const { return jerry_value_is_number(m_value); }
    bool IsUint32() const { return IsNumber() && (jerry_get_number_value (m_value) == (uint32_t) jerry_get_number_value (m_value)); }
    bool IsInt32() const { return IsNumber() && (jerry_get_number_value (m_value) == (int32_t) jerry_get_number_value (m_value)); }
    bool IsFunction() const { return jerry_value_is_function(m_value); }
    bool IsObject() const { return jerry_value_is_object(m_value); }
    bool IsSymbol() const { return jerry_value_is_symbol(m_value); }
    bool IsTypedArray() const { return jerry_value_is_typedarray(m_value); }
    bool IsArrayBuffer() const { return jerry_value_is_arraybuffer(m_value); }
    bool IsProxy() const { return false; }
    bool IsMap() const { return false; }
    bool IsMapIterator() const { return false; }
    bool IsSet() const { return false; }
    bool IsSetIterator() const { return false; }
    bool IsDate() const { return false; }
    bool IsRegExp() const { return false; }
    bool IsSharedArrayBuffer() const { return false; }
    bool IsAsyncFunction() const { return false; }
    bool IsNativeError() const { return false; }
    bool IsArrayBufferView() const { return jerry_value_is_typedarray(m_value) || jerry_value_is_dataview(m_value); }
    bool IsFloat64Array() const { return jerry_value_is_typedarray(m_value) && jerry_get_typedarray_type (m_value) == JERRY_TYPEDARRAY_FLOAT64; }
    bool IsUint8Array() const { return jerry_value_is_typedarray(m_value) && jerry_get_typedarray_type (m_value) == JERRY_TYPEDARRAY_UINT8; }
    bool IsDataView() const { return jerry_value_is_dataview(m_value); }

    double GetNumberValue(void) const { return jerry_get_number_value(m_value); }
    uint32_t GetUInt32Value(void) const { return (uint32_t)jerry_get_number_value(m_value); }
    int32_t GetInt32Value(void) const { return (int32_t)jerry_get_number_value(m_value); }
    int64_t GetInt64Value(void) const { return (int64_t)jerry_get_number_value(m_value); }
    bool GetBooleanValue(void) const { return jerry_get_boolean_value(m_value); }

    int GetStringLength(void) const { return jerry_get_string_length(m_value); }
    int GetStringUtf8Length(void) const { return jerry_get_utf8_string_length(m_value); }

    JerryValue* ToString(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_string(m_value));
    }

    JerryValue* ToInteger(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_number(m_value));
    }

    JerryValue* ToObject(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_object(m_value));
    }

    static JerryValue* NewExternal(void* ptr) {
        jerry_value_t object = jerry_create_object();
        jerry_set_object_native_pointer(object, ptr, &JerryV8ExternalTypeInfo);

        // TODO: move this to a better place so it'll be constructed once, maybe create a constructor and prototype?
        jerry_value_t conv_failer = BuildHelperMethod("", "this.toString = this.valueOf = function() { throw new TypeError('Invalid usage'); }");
        jerry_call_function(conv_failer, object, NULL, 0);
        jerry_release_value(conv_failer);

        return new JerryValue(object);
    }

    void* GetExternalData(void) const {
        void *native_p;
        bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8ExternalTypeInfo);
        if (!has_data) {
            return NULL;
        }

        return native_p;
    }

    bool IsExternal() const {
        void *native_p;
        bool has_data = jerry_get_object_native_pointer(m_value, &native_p, &JerryV8ExternalTypeInfo);
        return has_data;
    }

    JerryValue* Copy() const {
        return new JerryValue(jerry_acquire_value(m_value));
    }

    JerryV8InternalFieldData* GetInternalFieldData(int idx) {
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

    void SetInternalField(int idx, JerryValue* value) {
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

    void SetInternalField(int idx, void* value) {
        JerryV8InternalFieldData* data = GetInternalFieldData(idx);

        // TODO: maybe delete "$$internal_<x>" properties?
        data->fields[idx] = value;
        data->is_value[idx] = false;
    }

    template<typename T>
    T GetInternalField(int idx) {
        JerryV8InternalFieldData* data = GetInternalFieldData(idx);
        if (data == NULL) {
            return NULL;
        }

        if (std::is_same<T, JerryValue*>::value) {
            if (!data->is_value[idx]) {
                return NULL;
            }

            std::string internal_name = "$$internal_" + idx;
            JerryValue name(jerry_create_string_from_utf8((const jerry_char_t*)internal_name.c_str()));
            return this->GetProperty(&name);
        } else if (std::is_same<T, void*>::value) {
            return reinterpret_cast<T>(data->fields[idx]);
        } else {
            // DON'T do this...
            abort();
        }
    }

    int InternalFieldCount(void) {
        JerryV8InternalFieldData *data;
        bool has_data = jerry_get_object_native_pointer(m_value, reinterpret_cast<void**>(&data), &JerryV8InternalFieldTypeInfo);
        if (!has_data) {
            return 0;
        }

        return data->count;
    }

    JerryValue(JerryValue& that) = delete;

private:

    jerry_value_t m_value;
};

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

    void InstallProperties(const jerry_value_t target) {
        for (PropertyEntry* prop : m_properties) {
            // TODO: do not ignore the prop->attributes
            jerry_value_t result = jerry_create_undefined();

            switch (prop->type) {
                case PropertyEntry::Value: {
                    result = jerry_set_property(target, prop->key->value(), prop->value->value());
                    break;
                }
                case PropertyEntry::GetterSetter: {
                    jerry_property_descriptor_t desc;
                    jerry_init_property_descriptor_fields(&desc);

                    // add getter
                    if (prop->value != NULL) {
                        desc.is_get_defined = true;
                        desc.getter = jerry_acquire_value(prop->value->value());
                    }

                    // add setter
                    if (prop->setter != NULL) {
                        desc.is_set_defined = true;
                        desc.setter = jerry_acquire_value(prop->setter->value());
                    }

                    result = jerry_define_own_property(target, prop->key->value(), &desc);

                    jerry_free_property_descriptor_fields(&desc);
                    break;
                }
            }

            bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
            /* TODO: check isOK? */
            jerry_release_value(result);
        }
    }

protected:
    JerryTemplate(JerryHandle::Type type)
        : JerryHandle(type)
    {
    }

    void ReleaseProperties(void) {
        for (PropertyEntry* prop : m_properties) {
            delete prop;
        }
    }

    std::vector<PropertyEntry*> m_properties;
};


static jerry_value_t JerryV8GetterSetterHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt);

/* Getter/setter call types & methods */
union AccessorEntryGetter {
    v8::AccessorGetterCallback stringed;
    v8::AccessorNameGetterCallback named;
};

union AccessorEntrySetter {
    v8::AccessorSetterCallback stringed;
    v8::AccessorNameSetterCallback named;
};

struct JerryV8GetterSetterHandlerData {
    bool is_setter;
    bool is_named;
    union {
        AccessorEntryGetter getter;
        AccessorEntrySetter setter;
    } v8;
    jerry_value_t external;
};

static void JerryV8GetterSetterHandlerDataFree(void *data_p) {
    JerryV8GetterSetterHandlerData* data = reinterpret_cast<JerryV8GetterSetterHandlerData*>(data_p);

    if (data->is_setter == false && data->external) {
        jerry_release_value(data->external);
    }

    delete data;
}

static jerry_object_native_info_t JerryV8GetterSetterHandlerTypeInfo = {
    .free_cb = JerryV8GetterSetterHandlerDataFree,
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
        , data(data)
        , settings(settings)
        , attribute(attribute)
        , is_named(false)
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
        , data(data)
        , settings(settings)
        , attribute(attribute)
        , is_named(true)
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

class JerryObjectTemplate : public JerryTemplate {
public:
    JerryObjectTemplate()
        : JerryTemplate(ObjectTemplate)
        , m_function_template(NULL)
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

    static bool SetAccessor(const jerry_value_t target, AccessorEntry& entry) {
        jerry_property_descriptor_t prop_desc;
        jerry_init_property_descriptor_fields (&prop_desc);

        // TODO: is there always a getter?
        prop_desc.is_get_defined = true;
        prop_desc.getter = jerry_create_external_function(JerryV8GetterSetterHandler);

        {
            JerryV8GetterSetterHandlerData* data = new JerryV8GetterSetterHandlerData();
            data->v8.getter = entry.getter;
            data->external = NULL; // TODO
            data->is_setter = false;

            jerry_set_object_native_pointer(prop_desc.getter, data, &JerryV8GetterSetterHandlerTypeInfo);
        }

        prop_desc.is_set_defined = (entry.setter.stringed != NULL);
        if (prop_desc.is_set_defined) {
            prop_desc.setter = jerry_create_external_function(JerryV8GetterSetterHandler); // TODO: connect setter callback;

            JerryV8GetterSetterHandlerData* data = new JerryV8GetterSetterHandlerData();
            data->v8.setter = entry.setter;
            data->external = NULL; // TODO
            data->is_setter = true;

            jerry_set_object_native_pointer(prop_desc.setter, data, &JerryV8GetterSetterHandlerTypeInfo);
        }

        // TODO: handle settings and attributes
        jerry_value_t define_result = jerry_define_own_property(target, entry.name->value(), &prop_desc);
        bool isOk = !jerry_value_is_error(define_result) && jerry_get_boolean_value(define_result);
        jerry_release_value(define_result);

        jerry_free_property_descriptor_fields(&prop_desc);

        return isOk;
    }

    void InstallProperties(const jerry_value_t target) {
        JerryTemplate::InstallProperties(target);

        for (AccessorEntry* entry : m_accessors) {
            JerryObjectTemplate::SetAccessor(target, *entry);
        }

        if (m_internal_field_count) {
            JerryV8InternalFieldData *data = new JerryV8InternalFieldData(m_internal_field_count);
            jerry_set_object_native_pointer(target, data, &JerryV8InternalFieldTypeInfo);
        }
    }

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
};

static void JerryV8FunctionHandlerDataFree(void* data) {
    delete reinterpret_cast<JerryV8FunctionHandlerData*>(data);
}

static jerry_object_native_info_t JerryV8FunctionHandlerTypeInfo = {
    .free_cb = JerryV8FunctionHandlerDataFree,
};

static jerry_object_native_info_t JerryV8ObjectConstructed = {
    .free_cb = NULL,
};

static jerry_value_t JerryV8FunctionHandler(
    const jerry_value_t function_obj,
    const jerry_value_t this_val,
    const jerry_value_t args_p[],
    const jerry_length_t args_cnt);

class JerryFunctionTemplate : public JerryTemplate {
public:
    JerryFunctionTemplate()
        : JerryTemplate(FunctionTemplate)
        , m_prototype_template(NULL)
        , m_instance_template(NULL)
        , m_external(jerry_create_undefined())
        , m_function(NULL)
    {
    }

    ~JerryFunctionTemplate(void) {
        ReleaseProperties();
        jerry_release_value(m_external);

        delete m_function;
    }

    JerryObjectTemplate* PrototypeTemplate(void);
    JerryObjectTemplate* InstanceTemplate(void);

    bool HasInstanceTemplate(void) const { return m_instance_template != NULL; }

    void SetCallback(v8::FunctionCallback callback) { m_callback = callback; }
    void SetExternalData(jerry_value_t value) { m_external = value; }

    v8::FunctionCallback callback(void) const { return m_callback; }
    jerry_value_t external(void) const { return m_external; }

    JerryValue* GetFunction(void) {
        if (m_function == NULL) {
            // TODO: have function per context
            jerry_value_t jfunction = jerry_create_external_function(JerryV8FunctionHandler);

            JerryV8FunctionHandlerData* data = new JerryV8FunctionHandlerData();
            data->function_template = this; // Required so we can do instance checks.
            data->v8callback = m_callback;

            jerry_set_object_native_pointer(jfunction, data, &JerryV8FunctionHandlerTypeInfo);

            InstallProperties(jfunction);

            m_function = new JerryValue(jfunction);
        }

        return m_function;
    }

private:
    JerryObjectTemplate* m_prototype_template;
    JerryObjectTemplate* m_instance_template;
    v8::FunctionCallback m_callback;
    jerry_value_t m_external;
    JerryValue* m_function;
};

class JerryPlatform : public v8::Platform {
public:
    virtual void CallOnBackgroundThread(v8::Task*, v8::Platform::ExpectedRuntime) {}
    virtual void CallOnForegroundThread(v8::Isolate*, v8::Task*) {}
    virtual void CallDelayedOnForegroundThread(v8::Isolate*, v8::Task*, double) {}
    virtual double MonotonicallyIncreasingTime() { return 0.0; }
    virtual v8::TracingController* GetTracingController() { return NULL; }
};

class JerryHandleScope {
public:
    JerryHandleScope() {}

    ~JerryHandleScope();

    void AddHandle(JerryHandle* jvalue) {
        m_handles.push_back(jvalue);
    }

    void RemoveHandle(JerryHandle* jvalue) {
        // TODO: check if it really exists
        m_handles.erase(std::find(m_handles.begin(), m_handles.end(), jvalue));
    }

private:
    std::vector<JerryHandle*> m_handles;
};

class JerryIsolate {
public:
    JerryIsolate(const v8::Isolate::CreateParams& params)
    {
        jerry_init(JERRY_INIT_EMPTY/* | JERRY_INIT_MEM_STATS*/);
        jerry_port_default_set_abort_on_fail(true);

        {/* new Map() method */
            const char* fn_args = "";
            const char* fn_body = "return new Map();";
            m_fn_map_new = new JerryValue(BuildHelperMethod(fn_args, fn_body));
        }
        {/* isMap helper method */
            const char* fn_args = "value";
            const char* fn_body = "return value instanceof Map;";
            m_fn_is_map = new JerryValue(BuildHelperMethod(fn_args, fn_body));
        }
        {/* isSet helper method */
            const char* fn_args = "value";
            const char* fn_body = "return value instanceof Set;";
            m_fn_is_set = new JerryValue(BuildHelperMethod(fn_args, fn_body));
        }
        {/* Map.Set helper method */
            const char* fn_args = "map, key, value";
            const char* fn_body = "return map.set(key, value);";

            m_fn_map_set = new JerryValue(BuildHelperMethod(fn_args, fn_body));
        }

        {/* Object.assing helper method */
            const char* fn_args = "value";
            const char* fn_body = "return Object.assing({}, value);";

            m_fn_object_assign = new JerryValue(BuildHelperMethod(fn_args, fn_body));
        }

        InitalizeSlots();
    }

    ~JerryIsolate() {}

    const JerryValue& HelperMapNew(void) const { return *m_fn_map_new; }
    const JerryValue& HelperMapSet(void) const { return *m_fn_map_set; }
    const JerryValue& HelperObjectAssign(void) const { return *m_fn_object_assign; }
    const JerryValue& HelperIsMap(void) const { return *m_fn_is_map; }
    const JerryValue& HelperIsSet(void) const { return *m_fn_is_set; }

    void Enter(void) {
        JerryIsolate::s_currentIsolate = this;
    }

    void Exit(void) {
        JerryIsolate::s_currentIsolate = nullptr;
    }

    void Dispose(void) {
        for (std::vector<JerryTemplate*>::reverse_iterator it = m_templates.rbegin();
            it != m_templates.rend();
            it++) {
            JerryHandle* jhandle = *it;
            switch (jhandle->type()) {
                case JerryHandle::FunctionTemplate: delete reinterpret_cast<JerryFunctionTemplate*>(jhandle); break;
                case JerryHandle::ObjectTemplate: delete reinterpret_cast<JerryObjectTemplate*>(jhandle); break;
                default:
                    fprintf(stderr, "Isolate::Dispose unsupported type (%d)\n", jhandle->type());
                    break;
            }
        }

        delete m_fn_map_new;
        delete m_fn_is_map;
        delete m_fn_is_set;
        delete m_fn_map_set;
        delete m_fn_object_assign;

        // Release slots
        {
            int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiPointerSize;
            delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex]);
            delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex]);
            delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex]);
            delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex]);
            delete reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex]);
        }

        jerry_cleanup();

        // Warning!... Do not use the JerryIsolate after this!
        // If you do: dragons will spawn from the depths of the earth and tear everything apart!
        // You have been warned!
        delete this;
    }

    void PushHandleScope(void* ptr) {
        m_handleScopes.push(new JerryHandleScope());
    }

    void PopHandleScope(void* ptr) {
        JerryHandleScope* handleScope = m_handleScopes.top();
        m_handleScopes.pop();

        delete handleScope;
    }

    JerryHandleScope* CurrentHandleScope(void) {
        return m_handleScopes.top();
    }

    void AddToHandleScope(JerryHandle* jvalue) {
        m_handleScopes.top()->AddHandle(jvalue);
    }

    void AddTemplate(JerryTemplate* handle) {
        // TODO: make the vector a set or a map
        if (std::find(std::begin(m_templates), std::end(m_templates), handle) == std::end(m_templates)) {
            m_templates.push_back(handle);
        }
    }

    static v8::Isolate* toV8(JerryIsolate* iso) {
        return reinterpret_cast<v8::Isolate*>(iso);
    }

    static JerryIsolate* fromV8(v8::Isolate* iso) {
        return reinterpret_cast<JerryIsolate*>(iso);
    }

    static JerryIsolate* fromV8(v8::internal::Isolate* iso) {
        return reinterpret_cast<JerryIsolate*>(iso);
    }

    static JerryIsolate* GetCurrent(void) {
        return JerryIsolate::s_currentIsolate;
    }

private:
    void InitalizeSlots(void) {
        ::memset(m_slot, 0, sizeof(m_slot));

        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiPointerSize;
        m_slot[v8::internal::Internals::kExternalMemoryOffset / v8::internal::kApiPointerSize] = (void*) 0;
        m_slot[v8::internal::Internals::kExternalMemoryLimitOffset / v8::internal::kApiPointerSize] = (void*) (1024*1024);
                    // v8::internal::kExternalAllocationSoftLimit

        // Undefined
        m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex] = new JerryValue(jerry_create_undefined());
        // Null
        m_slot[root_offset + v8::internal::Internals::kNullValueRootIndex] = new JerryValue(jerry_create_null());
        // Boolean True
        m_slot[root_offset + v8::internal::Internals::kTrueValueRootIndex] = new JerryValue(jerry_create_boolean(true));
        // Boolean False
        m_slot[root_offset + v8::internal::Internals::kFalseValueRootIndex] = new JerryValue(jerry_create_boolean(false));
        // Empty string
        m_slot[root_offset + v8::internal::Internals::kEmptyStringRootIndex] = new JerryValue(jerry_create_string_sz((const jerry_char_t*)"", 0));
        // TODO: do we need these?
        //m_slot[root_offset + v8::internal::Internals::kInt32ReturnValuePlaceholderIndex] =
        //m_slot[root_offset + v8::internal::Internals::kUint32ReturnValuePlaceholderIndex] =
        //m_slot[root_offset + v8::internal::Internals::kDoubleReturnValuePlaceholderIndex] =
    }

    // Slots accessed by v8::Isolate::Get/SetData
    // They must be the first field of GraalIsolate
    void* m_slot[22] = {};

    std::stack<JerryHandleScope*> m_handleScopes;
    std::vector<JerryTemplate*> m_templates;
    JerryValue* m_fn_map_new;
    JerryValue* m_fn_is_map;
    JerryValue* m_fn_is_set;
    JerryValue* m_fn_map_set;
    JerryValue* m_fn_object_assign;

    static JerryIsolate* s_currentIsolate;
};

JerryIsolate* JerryIsolate::s_currentIsolate = nullptr;

class JerryContext : public JerryHandle {
public:

    JerryContext(JerryIsolate* iso)
        : JerryHandle(JerryHandle::Context)
        , m_isolate(iso)
    {}

    JerryIsolate* GetIsolate(void) {
        return m_isolate;
    }

    void Enter(void) {
        m_isolate->Enter();
    }

    void Exit(void) {
        m_isolate->Exit();
    }

    static v8::Context* toV8(JerryContext* ctx) {
        return reinterpret_cast<v8::Context*>(ctx);
    }

    static JerryContext* fromV8(v8::Context* ctx) {
        return reinterpret_cast<JerryContext*>(ctx);
    }

private:
    JerryIsolate* m_isolate;
};


template<typename T>
class JerryPropertyCallbackInfo : public v8::PropertyCallbackInfo<T> {
/* From parent:
  static const int kShouldThrowOnErrorIndex = 0;
  static const int kHolderIndex = 1;
  static const int kIsolateIndex = 2;
  static const int kReturnValueDefaultValueIndex = 3;
  static const int kReturnValueIndex = 4;
  static const int kDataIndex = 5;
  static const int kThisIndex = 6;
  static const int kArgsLength = 7;

*/
public:
    static const int kImplicitArgsSize = v8::FunctionCallbackInfo<T>::kArgsLength + 1;

    JerryPropertyCallbackInfo(
        const jerry_value_t function_obj,
        const jerry_value_t this_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt,
        const JerryV8GetterSetterHandlerData* data)
        : v8::PropertyCallbackInfo<T>(reinterpret_cast<v8::internal::Object**>(BuildArgs(this_val, data)))
        , m_args(reinterpret_cast<JerryHandle**>(this->args_))
    {
    }

    ~JerryPropertyCallbackInfo() {
        delete [] m_args;
    }

private:
    static JerryHandle** BuildArgs(const jerry_value_t this_val, const JerryV8GetterSetterHandlerData* data) {
        JerryHandle **values = new JerryHandle*[kImplicitArgsSize];

        values[v8::PropertyCallbackInfo<T>::kShouldThrowOnErrorIndex] = 0; // TODO: fix this
        values[v8::PropertyCallbackInfo<T>::kHolderIndex] = new JerryValue(jerry_acquire_value(this_val)); // TODO: 'this' object is not correct
        // TODO: correctly fill the arguments:
        values[v8::PropertyCallbackInfo<T>::kIsolateIndex] = reinterpret_cast<JerryHandle*>(v8::Isolate::GetCurrent()); /* isolate; */
        //values[v8::PropertyCallbackInfo<T>::kReturnValueDefaultValueIndex] = new JerryValue(jerry_create_undefined()); // TODO
        values[v8::PropertyCallbackInfo<T>::kReturnValueIndex] = new JerryValue(jerry_create_undefined());

        values[v8::PropertyCallbackInfo<T>::kDataIndex] = new JerryValue(jerry_acquire_value(data->external)); /* data; */
        values[v8::PropertyCallbackInfo<T>::kThisIndex] = new JerryValue(jerry_acquire_value(this_val));
        values[v8::PropertyCallbackInfo<T>::kArgsLength] = 0; // TODO

        // HandleScope will do the cleanup for us at a later stage
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kHolderIndex]);
        //JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kReturnValueDefaultValueIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kReturnValueIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kDataIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kThisIndex]);

        return values;
    }

    JerryHandle** m_args;
};

static jerry_value_t JerryV8GetterSetterHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt) {
    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(function_obj, &native_p, &JerryV8GetterSetterHandlerTypeInfo);

    if (!has_data) {
        fprintf(stderr, "ERRR.... should not be here!(%s:%d)\n", __FILE__, __LINE__);
        abort();
        return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t*)"BAD!!");
    }
    JerryV8GetterSetterHandlerData* data = reinterpret_cast<JerryV8GetterSetterHandlerData*>(native_p);

    // Make sure that Localy allocated vars will be freed upon exit.
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

    jerry_value_t jret = jerry_create_undefined();

    if (data->is_setter) {
        JerryPropertyCallbackInfo<void> info(function_obj, this_val, args_p, args_cnt, data);

        // TODO: assert on args[0]?
        JerryValue new_value(jerry_acquire_value(args_p[0]));
        if (data->is_named) {
            data->v8.setter.named(v8::Local<v8::Name>(), *reinterpret_cast<v8::Local<v8::Value>*>(&new_value), info);
        } else {
            data->v8.setter.stringed(v8::Local<v8::String>(), *reinterpret_cast<v8::Local<v8::Value>*>(&new_value), info);
        }
    } else {
        JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, data);
        if (data->is_named) {
            data->v8.getter.named(v8::Local<v8::Name>(), info);
        } else {
            data->v8.getter.stringed(v8::Local<v8::String>(), info);
        }

        v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

        // Again: dragons!
        JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

        jret = jerry_acquire_value(retVal->value());
    }


    // No need to delete the JerryValue here, the HandleScope will take (should) care of it!
    //delete retVal;

    return jret;
}



/* Function callback handler */
template<typename T>
class JerryFunctionCallbackInfo : public v8::FunctionCallbackInfo<T> {
/* Form parent:
  static const int kHolderIndex = 0;
  static const int kIsolateIndex = 1;
  static const int kReturnValueDefaultValueIndex = 2;
  static const int kReturnValueIndex = 3;
  static const int kDataIndex = 4;
  static const int kCalleeIndex = 5;
  static const int kContextSaveIndex = 6;
static const int kNewTargetIndex = 7;
*/
public:
    static const int kImplicitArgsSize = v8::FunctionCallbackInfo<T>::kNewTargetIndex + 1;

    JerryFunctionCallbackInfo(
        const jerry_value_t function_obj,
        const jerry_value_t this_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt,
        const JerryV8FunctionHandlerData* handlerData)
        // Please note there is a "Hack"/"Fix" in the v8.h file where the FunctionCallbackInfo's "values_" member is accessed!
        : v8::FunctionCallbackInfo<T>(reinterpret_cast<v8::internal::Object**>(BuildImplicitArgs(function_obj, this_val, handlerData)),
                                      reinterpret_cast<v8::internal::Object**>(BuildArgs(this_val, args_p, args_cnt) + args_cnt - 1),
                                      args_cnt)
        // Beware of magic: !!!
        , m_values(reinterpret_cast<JerryHandle**>(this->values_) - args_cnt + 1)
    {}

    ~JerryFunctionCallbackInfo() {
        const int args_count = this->Length() + 1; /* +1 is the js 'this' */
        for (int idx = 0; idx < args_count; idx++) {
            delete m_values[idx];
        }
        delete [] m_values;

        JerryHandle **implicit_args = reinterpret_cast<JerryHandle**>(this->implicit_args_);
        delete [] implicit_args;
    }

private:
    static JerryHandle** BuildArgs(
        const jerry_value_t this_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt) {

        JerryHandle **values = new JerryHandle*[args_cnt + 1 /* this */];
        /* this should be at 'values - 1' */

        /* args_p[0]  is at 'values - 0' */
        /* args_p[1]  is at 'values - 1' */
        /* args_p[2]  is at 'values - 2' */
        for (jerry_length_t idx = 0; idx < args_cnt; idx++) {
            values[args_cnt - idx - 1] = new JerryValue(args_p[idx]);
        }
        values[args_cnt] = new JerryValue(this_val);

        return values;
    }

    static JerryHandle** BuildImplicitArgs(const jerry_value_t function_obj,
                                           const jerry_value_t this_val,
                                           const JerryV8FunctionHandlerData* handlerData) {
        JerryHandle **values = new JerryHandle*[kImplicitArgsSize];
        /* TODO: From the docs:
         * If the callback was created without a Signature, this is the same
         * value as This(). If there is a signature, and the signature didn't match
         * This() but one of its hidden prototypes, this will be the respective
         * hidden prototype.
         */
        values[v8::FunctionCallbackInfo<T>::kHolderIndex] = new JerryValue(jerry_acquire_value(this_val));
        // TODO: correctly fill the arguments:
        values[v8::FunctionCallbackInfo<T>::kIsolateIndex] = reinterpret_cast<JerryHandle*>(v8::Isolate::GetCurrent()); /* isolate; */
        values[v8::FunctionCallbackInfo<T>::kReturnValueIndex] = new JerryValue(jerry_create_undefined()); /* construct_call ? this : nullptr;*/

        values[v8::FunctionCallbackInfo<T>::kDataIndex] = new JerryValue(jerry_acquire_value(handlerData->function_template->external())); /* data; */
        values[v8::FunctionCallbackInfo<T>::kCalleeIndex] = NULL; /* callee; */
        values[v8::FunctionCallbackInfo<T>::kNewTargetIndex] = NULL; /* new_target; */

        // HandleScope will do the cleanup for us at a later stage
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::FunctionCallbackInfo<T>::kHolderIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::FunctionCallbackInfo<T>::kReturnValueIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::FunctionCallbackInfo<T>::kDataIndex]);

        return values;
    }

    JerryHandle** m_values;
};

static jerry_value_t JerryV8FunctionHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt) {

    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(function_obj, &native_p, &JerryV8FunctionHandlerTypeInfo);

    if (!has_data) {
        fprintf(stderr, "ERRR.... should not be here!(%s:%d)\n", __FILE__, __LINE__);
        abort();
        return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t*)"BAD!!");
    }
    JerryV8FunctionHandlerData* data = reinterpret_cast<JerryV8FunctionHandlerData*>(native_p);

    // Make sure that Localy allocated vars will be freed upon exit.
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

    // If this is a constructor call do a few extra things
    // TODO: this should be really checked correcty
    if (jerry_value_is_object(this_val)) {

        // TODO: remove this "constructor" call check.
        if (!jerry_get_object_native_pointer(this_val, NULL, &JerryV8ObjectConstructed)) {
            // Add reference to the function template
            // TODO: really do a correct is constructor call check and only set the constructor function template on the new instance.
            //jerry_set_object_native_pointer(this_val, data, &JerryV8FunctionHandlerTypeInfo);

            if (data->function_template->HasInstanceTemplate()) {
                JerryObjectTemplate* tmplt = data->function_template->InstanceTemplate();
                tmplt->InstallProperties(this_val);
            }

            // The method was called with a "this" object and there was no "JerryV8ObjectConstructed" set
            // assume that this is a constructo call.
            // TODO: this is not always correct logic. Propert constructor call check is needed.
            jerry_set_object_native_pointer(this_val, data, &JerryV8ObjectConstructed);
        }
    }

    jerry_value_t jret = jerry_create_undefined();

    if (data->v8callback != NULL) {
        JerryFunctionCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, data);

        data->v8callback(info);

        v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

        // Again: dragons!
        JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

        jret = jerry_acquire_value(retVal->value());
    }

    // No need to delete the JerryValue here, the HandleScope will take (should) care of it!
    //delete retVal;

    return jret;
}


/**/

JerryHandleScope::~JerryHandleScope(void) {
    for (std::vector<JerryHandle*>::reverse_iterator it = m_handles.rbegin();
        it != m_handles.rend();
        it++) {
        JerryHandle* jhandle = *it;
        switch (jhandle->type()) {
            case JerryHandle::Value: delete reinterpret_cast<JerryValue*>(jhandle); break;
            case JerryHandle::Context: delete reinterpret_cast<JerryContext*>(jhandle); break;
            // FunctionTemplate and ObjectTemplates are now Isolate level items.
            default: fprintf(stderr, "~JerryHandleScope::Unsupported handle type (%d)\n", jhandle->type()); break;
        }
    }
}

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
        JerryIsolate::GetCurrent()->AddTemplate(m_instance_template);
    }

    return m_instance_template;
}


/* V8 API helpers */
#define RETURN_HANDLE(T, ISOLATE, HANDLE) \
do {                                                                    \
    JerryHandle *__handle = HANDLE;                                     \
    return v8::Local<T>::New(ISOLATE, reinterpret_cast<T*>(&__handle)); \
} while (0)


/* API Implementation */
namespace v8 {

namespace platform {

    Platform* CreateDefaultPlatform(
        int thread_pool_size,
        IdleTaskSupport idle_task_support,
        InProcessStackDumping in_process_stack_dumping,
        v8::TracingController* tracing_controller) {
        return new JerryPlatform();
    }

} // namespace platform

/* base mutex*/
namespace base {
class Mutex {
public:
    Mutex() { }
    ~Mutex() { }
};

} // namespace base

const char* V8::GetVersion() {
    return "JerryScript v2.0";
}

/* V8 statics */
bool V8::InitializeICUDefaultLocation(const char* exec_path, const char* icu_data_file) {
    return true;
}

void V8::InitializeExternalStartupData(char const*) { }

void V8::InitializePlatform(v8::Platform*) { }

bool V8::Initialize() {
    return true;
}

bool V8::Dispose() {
    return true;
}

void V8::ToLocalEmpty() { }

void V8::FromJustIsNothing() { }

void V8::ShutdownPlatform() { }

/* ArrayBuffer & Allocator */
Local<ArrayBuffer> ArrayBuffer::New(Isolate* isolate, void* data, size_t byte_length, ArrayBufferCreationMode mode) {
    jerry_value_t buffer;

    if (mode == ArrayBufferCreationMode::kInternalized) {
        buffer = jerry_create_arraybuffer(byte_length);
        jerry_arraybuffer_write(buffer, 0, (uint8_t*)data, byte_length);
    } else {
        buffer = jerry_create_arraybuffer_external(byte_length, (uint8_t*)data, nullptr);
    }

    RETURN_HANDLE(ArrayBuffer, isolate, new JerryValue(buffer));
}

ArrayBuffer::Contents ArrayBuffer::GetContents() {
    JerryValue* jbuffer = reinterpret_cast<JerryValue*> (this);
    jerry_value_t buffer = jbuffer->value();

    ArrayBuffer::Contents contents;
    contents.data_ = (void*) jerry_get_arraybuffer_pointer (buffer);
    contents.byte_length_ = (size_t) jerry_get_arraybuffer_byte_length (buffer);

    return contents;
}

size_t ArrayBuffer::ByteLength() const {
    const JerryValue* jbuffer = reinterpret_cast<const JerryValue*> (this);

    return (size_t) jerry_get_arraybuffer_byte_length (jbuffer->value());
}

void ArrayBuffer::Neuter() {
}

void ArrayBuffer::Allocator::SetProtection(void* data, size_t length, Protection protection) { }

void* ArrayBuffer::Allocator::Reserve(size_t length) {
    return nullptr;
}

void ArrayBuffer::Allocator::Free(void* data, size_t length, AllocationMode mode) { }

ArrayBuffer::Allocator* ArrayBuffer::Allocator::NewDefaultAllocator() {
    return nullptr;
}

size_t TypedArray::Length() {
    return (size_t) jerry_get_typedarray_length (reinterpret_cast<JerryValue*> (this)->value());
}

size_t ArrayBufferView::ByteLength() {
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;
    jerry_length_t byteLength = 0;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), NULL, &byteLength);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), NULL, &byteLength);
    } else {
        printf("Unknown object...\n");
    }

    jerry_release_value(buffer);

    return byteLength;
}

size_t ArrayBufferView::ByteOffset() {
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;
    jerry_length_t byteOffset = 0;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), &byteOffset, NULL);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), &byteOffset, NULL);
    } else {
        printf("Unknown object...\n");
    }

    jerry_release_value(buffer);
    return byteOffset;
}

Local<ArrayBuffer> ArrayBufferView::Buffer() {
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), NULL, NULL);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), NULL, NULL);
    } else {
        printf("Unknown object...\n");
    }

    RETURN_HANDLE(ArrayBuffer, Isolate::GetCurrent(), new JerryValue(buffer));
}

#define ArrayBufferView(view_class, view_type) \
    Local<view_class> view_class::New(Local<ArrayBuffer> array_buffer, size_t byte_offset, size_t length) { \
        JerryValue* jarraybuffer = reinterpret_cast<JerryValue*> (*array_buffer); \
        jerry_value_t arrayview = jerry_create_typedarray_for_arraybuffer_sz (view_type, jarraybuffer->value(), byte_offset, length); \
        if (jerry_value_is_error(arrayview)) { \
            printf("Error at Typedarray creation...\n"); \
        } \
        RETURN_HANDLE(view_class, Isolate::GetCurrent(), new JerryValue(arrayview)); \
    }

ArrayBufferView(Uint8Array, JERRY_TYPEDARRAY_UINT8);
ArrayBufferView(Uint32Array, JERRY_TYPEDARRAY_UINT32);
ArrayBufferView(Float64Array, JERRY_TYPEDARRAY_FLOAT64);

/* Isolate */
ResourceConstraints::ResourceConstraints() { }

Isolate* Isolate::New(const CreateParams& params) {
    return JerryIsolate::toV8(new JerryIsolate(params));
}

Isolate* Isolate::GetCurrent() {
    return JerryIsolate::toV8(JerryIsolate::GetCurrent());
}

void Isolate::LowMemoryNotification(void) { }

bool Isolate::IsDead() {
    return false;
}

void Isolate::Enter() {
    JerryIsolate::fromV8(this)->Enter();
}

void Isolate::Exit() {
    JerryIsolate::fromV8(this)->Exit();
}

void Isolate::Dispose() {
    JerryIsolate::fromV8(this)->Dispose();
}

void Isolate::GetHeapStatistics(HeapStatistics*) { }

HeapProfiler* Isolate::GetHeapProfiler() {
    return NULL;
}

/* Context */
Local<Context> Context::New(Isolate* isolate,
                            ExtensionConfiguration* extensions /*= NULL*/,
                            MaybeLocal<ObjectTemplate> global_template /*= MaybeLocal<ObjectTemplate>()*/,
                            MaybeLocal<Value> global_object /*= MaybeLocal<Value>()*/) {
    RETURN_HANDLE(Context, isolate, new JerryContext(JerryIsolate::fromV8(isolate)));
}

Isolate* Context::GetIsolate() {
    return JerryIsolate::toV8(JerryContext::fromV8(this)->GetIsolate());
}

void Context::Enter() {
    JerryContext::fromV8(this)->Enter();
}

void Context::Exit() {
    JerryContext::fromV8(this)->Exit();
}

Local<Object> Context::Global() {
    RETURN_HANDLE(Object, GetIsolate(), new JerryValue(jerry_get_global_object()));
}

/* HandleScope */
HandleScope::HandleScope(Isolate* isolate)
    : isolate_(reinterpret_cast<internal::Isolate*>(isolate))
{
    JerryIsolate::fromV8(isolate_)->PushHandleScope(this);
}

HandleScope::~HandleScope(void) {
    JerryIsolate::fromV8(isolate_)->PopHandleScope(this);
}

internal::Object** HandleScope::CreateHandle(internal::Isolate* isolate, internal::Object* value) {
    JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(value);
    switch (jhandle->type()) {
        case JerryHandle::FunctionTemplate:
        case JerryHandle::ObjectTemplate:
            reinterpret_cast<JerryIsolate*>(isolate)->AddTemplate(reinterpret_cast<JerryTemplate*>(jhandle));
            break;
        default:
            reinterpret_cast<JerryIsolate*>(isolate)->AddToHandleScope(jhandle);
            break;
    }
    return reinterpret_cast<internal::Object**>(jhandle);
}

/* Value */
Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
    return Just((int32_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value());
}

int32_t Value::Int32Value() const {
    return (int32_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value();
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
    return Just((uint32_t)reinterpret_cast<const JerryValue*>(this)->GetUInt32Value());
}

uint32_t Value::Uint32Value() const {
    return reinterpret_cast<const JerryValue*> (this)->GetUInt32Value();
}

Maybe<bool> Value::BooleanValue(Local<Context> context) const {
    return Just((bool)reinterpret_cast<const JerryValue*>(this)->GetBooleanValue());
}

bool Value::BooleanValue() const {
    return (bool)reinterpret_cast<const JerryValue*>(this)->GetBooleanValue();
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
    return Just((double)reinterpret_cast<const JerryValue*>(this)->GetNumberValue());
}

double Value::NumberValue() const {
    return (double)reinterpret_cast<const JerryValue*>(this)->GetNumberValue();
}

int64_t Value::IntegerValue() const {
    return (int64_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value();
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
    return Just((int64_t)reinterpret_cast<const JerryValue*>(this)->GetInt64Value());
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToString();
    RETURN_HANDLE(String, context->GetIsolate(), result);
}

Local<String> Value::ToString(Isolate* isolate) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToString();
    RETURN_HANDLE(String, isolate, result);
}

Local<Integer> Value::ToInteger(Isolate* isolate) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToInteger();
    RETURN_HANDLE(Integer, isolate, result);
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToObject();
    RETURN_HANDLE(Object, Isolate::GetCurrent(), result);
}

Local<Object> Value::ToObject(Isolate* isolate) const {
    JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToObject();
    RETURN_HANDLE(Object, isolate, result);
}

bool Value::IsBoolean() const {
    return reinterpret_cast<const JerryValue*> (this)->IsBoolean();
}

bool Value::IsFalse() const {
    return reinterpret_cast<const JerryValue*> (this)->IsFalse();
}

bool Value::IsTrue() const {
    return reinterpret_cast<const JerryValue*> (this)->IsTrue();
}

bool Value::IsPromise() const {
    return reinterpret_cast<const JerryValue*> (this)->IsPromise();
}

bool Value::IsArray() const {
    return reinterpret_cast<const JerryValue*> (this)->IsArray();
}

bool Value::IsObject() const {
    return reinterpret_cast<const JerryValue*> (this)->IsObject();
}

bool Value::IsNumber() const {
    return reinterpret_cast<const JerryValue*> (this)->IsNumber();
}

bool Value::IsUint32() const {
    return reinterpret_cast<const JerryValue*> (this)->IsUint32();
}

bool Value::IsInt32() const {
    return reinterpret_cast<const JerryValue*> (this)->IsInt32();
}

bool Value::IsFunction() const {
    return reinterpret_cast<const JerryValue*> (this)->IsFunction();
}

bool Value::IsSymbol() const {
    return reinterpret_cast<const JerryValue*> (this)->IsSymbol();
}

bool Value::IsTypedArray() const {
    return reinterpret_cast<const JerryValue*> (this)->IsTypedArray();
}

bool Value::IsArrayBuffer() const {
    return reinterpret_cast<const JerryValue*> (this)->IsArrayBuffer();
}

bool Value::IsProxy() const {
    return reinterpret_cast<const JerryValue*> (this)->IsProxy();
}

bool Value::IsMap() const {
    const JerryValue* jval = reinterpret_cast<const JerryValue*> (this);

    jerry_value_t is_map = JerryIsolate::fromV8(v8::Isolate::GetCurrent())->HelperIsMap().value();
    jerry_value_t arg = jval->value();
    jerry_value_t result = jerry_call_function(is_map, jerry_create_undefined(), &arg, 1);

    bool isMap = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isMap;
}

bool Value::IsMapIterator() const {
     return reinterpret_cast<const JerryValue*> (this)->IsMapIterator();
}

bool Value::IsSet() const {
    const JerryValue* jval = reinterpret_cast<const JerryValue*> (this);

    jerry_value_t is_set = JerryIsolate::fromV8(v8::Isolate::GetCurrent())->HelperIsSet().value();
    jerry_value_t arg = jval->value();
    jerry_value_t result = jerry_call_function(is_set, jerry_create_undefined(), &arg, 1);

    bool isSet = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isSet;
}

bool Value::IsSetIterator() const {
    return reinterpret_cast<const JerryValue*> (this)->IsSetIterator();
}

bool Value::IsDate() const {
    return reinterpret_cast<const JerryValue*> (this)->IsDate();
}

bool Value::IsRegExp() const {
    return reinterpret_cast<const JerryValue*> (this)->IsRegExp();
}

bool Value::IsSharedArrayBuffer() const {
    return reinterpret_cast<const JerryValue*> (this)->IsSharedArrayBuffer();
}

bool Value::IsAsyncFunction() const {
    return reinterpret_cast<const JerryValue*> (this)->IsAsyncFunction();
}

bool Value::IsNativeError() const {
    return reinterpret_cast<const JerryValue*> (this)->IsNativeError();
}

bool Value::IsArrayBufferView() const {
    return reinterpret_cast<const JerryValue*> (this)->IsArrayBufferView();
}

bool Value::IsFloat64Array() const {
    return reinterpret_cast<const JerryValue*> (this)->IsFloat64Array();
}

bool Value::IsUint8Array() const {
    return reinterpret_cast<const JerryValue*> (this)->IsUint8Array();
}

bool Value::IsDataView() const {
    return reinterpret_cast<const JerryValue*> (this)->IsDataView();
}

bool Value::IsExternal() const {
    return reinterpret_cast<const JerryValue*> (this)->IsExternal();
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
    const JerryValue* jValue = reinterpret_cast<const JerryValue*> (this);

    if (jValue->IsSymbol()) {
        jerry_value_t string_value = jerry_create_string ((const jerry_char_t *) "Symbol()");
        RETURN_HANDLE(String, context->GetIsolate(), new JerryValue(string_value));
    } else if (jValue->IsProxy()) {
        jerry_value_t string_value = jerry_create_string ((const jerry_char_t *) "[object Object]");
        RETURN_HANDLE(String, context->GetIsolate(), new JerryValue(string_value));
    }

    RETURN_HANDLE(String, context->GetIsolate(), jValue->ToString());
}

bool Value::Equals(Local<Value> value) const {
    const JerryValue* lhs = reinterpret_cast<const JerryValue*> (this);
    JerryValue* rhs = reinterpret_cast<JerryValue*> (*value);

    jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_EQUAL, lhs->value(), rhs->value());
    bool isEqual = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isEqual;
}

bool Value::StrictEquals(Local<Value> value) const {
    const JerryValue* lhs = reinterpret_cast<const JerryValue*> (this);
    JerryValue* rhs = reinterpret_cast<JerryValue*> (*value);

    jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_STRICT_EQUAL, lhs->value(), rhs->value());
    bool isEqual = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isEqual;
}

/* Integer */
Local<Integer> Integer::New(Isolate* isolate, int32_t value) {
    jerry_value_t result = jerry_create_number(value);
    RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

Local<Integer> Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
    jerry_value_t result = jerry_create_number((int32_t) value);
    RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

int64_t Integer::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetInt64Value();
}

/* Number */
Local<Number> Number::New(Isolate* isolate, double value) {
    jerry_value_t result = jerry_create_number(value);
    RETURN_HANDLE(Number, isolate, new JerryValue(result));
}

double Number::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetNumberValue();
}

/* UInt32 */
uint32_t Uint32::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetUInt32Value();
}

/* Int32 */
int32_t Int32::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetInt32Value();
}

/* Boolean */
bool Boolean::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetBooleanValue();
}

/* External */
/* External is a JS object */
Local<External> External::New(Isolate* isolate, void* value) {
    RETURN_HANDLE(External, isolate, JerryValue::NewExternal(value));
}

void* External::Value() const {
    return reinterpret_cast<const JerryValue*>(this)->GetExternalData();
}

/* Object */
Local<Object> Object::New(Isolate* isolate) {
    jerry_value_t obj = jerry_create_object();

    RETURN_HANDLE(Object, isolate, new JerryValue(obj));
}

Maybe<bool> Object::Set(Local<Context> context, Local<Value> key, Local<Value> value) {
    return Just(reinterpret_cast<JerryValue*>(this)->SetProperty(
                    reinterpret_cast<JerryValue*>(*key),
                    reinterpret_cast<JerryValue*>(*value)));
}

bool Object::Set(Local<Value> key, Local<Value> value) {
    return reinterpret_cast<JerryValue*>(this)->SetProperty(
                reinterpret_cast<JerryValue*>(*key),
                reinterpret_cast<JerryValue*>(*value));
}

Maybe<bool> Object::Set(Local<Context> context, uint32_t index, Local<Value> value) {
    return Just(reinterpret_cast<JerryValue*> (this)->SetPropertyIdx(index, reinterpret_cast<JerryValue*>(*value)));
}

bool Object::Set(uint32_t index, Local<Value> value) {
    return reinterpret_cast<JerryValue*> (this)->SetPropertyIdx(index, reinterpret_cast<JerryValue*>(*value));
}

Local<Value> Object::Get(uint32_t index) {
    RETURN_HANDLE(Value, Isolate::GetCurrent(), reinterpret_cast<JerryValue*> (this)->GetPropertyIdx(index));
}

MaybeLocal<Value> Object::Get(Local<Context> context, uint32_t index) {
    RETURN_HANDLE(Value, context->GetIsolate(), reinterpret_cast<JerryValue*> (this)->GetPropertyIdx(index));
}

Local<Value> Object::Get(Local<Value> key) {
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

    RETURN_HANDLE(Value, Isolate::GetCurrent(), reinterpret_cast<JerryValue*> (this)->GetProperty(jkey));
}

MaybeLocal<Value> Object::Get(Local<Context> context, Local<Value> key) {
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

    RETURN_HANDLE(Value, context->GetIsolate(), reinterpret_cast<JerryValue*> (this)->GetProperty(jkey));
}

Maybe<bool> Object::Delete(Local<Context> context, Local<Value> key) {
    JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
    JerryValue* jkey = reinterpret_cast<JerryValue*> (*key);

    return Just(jerry_delete_property (jobj->value(), jkey->value()));
}

Maybe<bool> Object::Has(Local<Context> context, Local<Value> key) {
    JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
    JerryValue* jkey = reinterpret_cast<JerryValue*> (*key);

    jerry_value_t has_prop_js = jerry_has_property (jobj->value(), jkey->value());
    bool has_prop = jerry_get_boolean_value (has_prop_js);
    jerry_release_value (has_prop_js);

    return Just(has_prop);
}

Maybe<bool> Object::DefineOwnProperty(Local<Context> context, Local<Name> key, Local<Value> value, PropertyAttribute attributes) {
    JerryValue* obj = reinterpret_cast<JerryValue*> (this);
    JerryValue* prop_name = reinterpret_cast<JerryValue*> (*key);
    JerryValue* prop_value = reinterpret_cast<JerryValue*> (*value);

    jerry_property_descriptor_t prop_desc = {
        .is_value_defined = true,
        .is_get_defined = false,
        .is_set_defined = false,
        .is_writable_defined = true,
        .is_writable = attributes & !PropertyAttribute::ReadOnly,
        .is_enumerable_defined = true,
        .is_enumerable = attributes & !PropertyAttribute::DontEnum,
        .is_configurable_defined = true,
        .is_configurable = attributes & !PropertyAttribute::DontDelete,
        .value = prop_value->value(),
        .getter = jerry_create_undefined(),
        .setter = jerry_create_undefined()
    };

    jerry_value_t result = jerry_define_own_property (obj->value(), prop_name->value(), &prop_desc);
    bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return Just(isOk);
}

Maybe<bool> Object::SetPrototype(Local<Context> context, Local<v8::Value> prototype) {
    return Just(SetPrototype(prototype));
}

bool Object::SetPrototype(Local<Value> prototype) {
    JerryValue* obj = reinterpret_cast<JerryValue*> (this);
    JerryValue* proto = reinterpret_cast<JerryValue*> (*prototype);

    jerry_value_t result = jerry_set_prototype (obj->value(), proto->value());
    bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
    jerry_release_value(result);

    return isOk;
}

Isolate* Object::GetIsolate() {
    return Isolate::GetCurrent();
}

Maybe<bool> Object::SetAccessor(Local<Context> context,
                                Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter, /* = 0 */
                                MaybeLocal<Value> data, /* = MaybeLocal<Value>()*/
                                AccessControl settings, /* = DEFAULT */
                                PropertyAttribute attribute /* = None */) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

    JerryValue* jdata = NULL;
    if (!data.IsEmpty()) {
        Local<Value> dataValue;
        bool isOk = data.ToLocal(&dataValue);
        (void)isOk; // the "emptyness" is alread checked this should be always true,
        // TODO: maybe assert on "isOK"?

        jdata = reinterpret_cast<JerryValue*>(*dataValue)->Copy();
    }

    AccessorEntry entry(
        reinterpret_cast<JerryValue*>(*name)->Copy(),
        getter,
        setter,
        jdata,
        settings,
        attribute
    );

    bool configured = JerryObjectTemplate::SetAccessor(jobj->value(), entry);
    return Just(configured);
}

void Object::SetInternalField(int idx, Local<Value> value) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    // Indexing starts from 0!

    jobj->SetInternalField(idx, reinterpret_cast<JerryValue*>(*value));
}

void Object::SetAlignedPointerInInternalField(int idx, void* value) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    // Indexing starts from 0!

    jobj->SetInternalField(idx, value);
}

int Object::InternalFieldCount(void) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    return jobj->InternalFieldCount();
}

void* Object::SlowGetAlignedPointerFromInternalField(int idx) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    return jobj->GetInternalField<void*>(idx);
}

Local<Value> Object::SlowGetInternalField(int idx) {
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    RETURN_HANDLE(Value, Isolate::GetCurrent(), jobj->GetInternalField<JerryValue*>(idx));
}

Local<Object> Object::Clone(void) {
    // shallow copy!
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

    // TODO: maybe move the helper calls to a concrete method
    jerry_value_t obj_assign = JerryIsolate::fromV8(GetIsolate())->HelperObjectAssign().value();
    jerry_value_t arg = jobj->value();
    jerry_value_t result = jerry_call_function(obj_assign, jerry_create_undefined(), &arg, 1);

    RETURN_HANDLE(Object, GetIsolate(), new JerryValue(result));
}

/* Array */
Local<Array> Array::New(Isolate* isolate, int length) {
    if (length < 0) {
        length = 0;
    }

    jerry_value_t array_value = jerry_create_array(length);
    RETURN_HANDLE(Array, isolate, new JerryValue(array_value));
}

uint32_t Array::Length() const {
    const JerryValue* array = reinterpret_cast<const JerryValue*>(this);

    return jerry_get_array_length(array->value());
}

/* Map */
Local<Map> Map::New(Isolate* isolate) {
    // TODO: add jerry api for map creation;
    jerry_value_t map_builder = JerryIsolate::fromV8(isolate)->HelperMapNew().value();
    jerry_value_t new_map = jerry_call_function(map_builder, jerry_create_undefined(), NULL, 0);

    RETURN_HANDLE(Map, isolate, new JerryValue(new_map));
}

MaybeLocal<Map> Map::Set(Local<Context> context, Local<Value> key, Local<Value> value) {
    JerryValue* jmap = reinterpret_cast<JerryValue*>(this);
    JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(*value);

    // TODO: ADD JS api / make sure the function is only created once.

    jerry_value_t map_set_helper = JerryIsolate::fromV8(context->GetIsolate())->HelperMapSet().value();
    jerry_value_t args[] = { jmap->value(), jkey->value(), jvalue->value() };
    jerry_value_t result = jerry_call_function(map_set_helper, jerry_create_undefined(), args, 3);
    jerry_release_value(result);

    return Local<Map>(this);
}

Local<Private> Private::New(Isolate* isolate, Local<String> name) {
    JerryValue* jname = reinterpret_cast<JerryValue*>(*name);

    RETURN_HANDLE(Private ,isolate, jname->Copy());
}

/* Symbol */
Local<Symbol> Symbol::New(Isolate* isolate, Local<String> name) {
    JerryValue* jname = reinterpret_cast<JerryValue*>(*name);

    jerry_value_t symbol_name = jerry_create_symbol (jname->value());
    RETURN_HANDLE(Symbol, isolate, new JerryValue(symbol_name));
}

/* Message */
Maybe<int> Message::GetStartColumn(v8::Local<v8::Context> context) const {
    return Just(0);
}

Maybe<int> Message::GetEndColumn(v8::Local<v8::Context> context) const {
    return Just(0);
}

int Message::GetLineNumber() const {
    return 0;
}

Local<Value> Message::GetScriptResourceName() const {
    return Local<Value>();
}

Local<String> Message::GetSourceLine() const {
    return Local<String>();
}

ScriptOrigin Message::GetScriptOrigin() const {
    return ScriptOrigin(Local<Value>());
}

/* String */
MaybeLocal<String> String::NewFromOneByte(
    Isolate* isolate, unsigned char const* data, v8::NewStringType type, int length /* = -1 */) {
    // TODO: what is the diff between the OneByte/TwoByte/Utf8 etc.?
    return String::NewFromUtf8(isolate, (const char*)data, (String::NewStringType)type, length);
}

Local<String> String::NewFromOneByte(
    Isolate* isolate, unsigned char const* data, String::NewStringType type /* = kNormalString */, int length /* = -1 */) {
    // TODO: what is the diff between the OneByte/TwoByte/Utf8 etc.?
    return String::NewFromUtf8(isolate, (const char*)data, (String::NewStringType)type, length);
}

Local<String> String::NewFromUtf8(
    Isolate* isolate, const char* data, NewStringType type /*= kNormalString*/, int length /*= -1*/) {
    if (length >= String::kMaxLength) {
        return Local<String>();
    }

    if (length == -1) {
        length = strlen(data);
    }

    jerry_value_t str_value = jerry_create_string_sz_from_utf8((const jerry_char_t*)data, length);
    RETURN_HANDLE(String, isolate, new JerryValue(str_value));
}

/** Allocates a new string from UTF-8 data. Only returns an empty value when
 * length > kMaxLength. **/
MaybeLocal<String> String::NewFromUtf8(
    Isolate* isolate, const char* data, v8::NewStringType type, int length /*= -1*/) {
    return String::NewFromUtf8(isolate, data, (String::NewStringType)type, length);
}

int String::WriteUtf8(char* buffer, int length, int* nchars_ref, int options) const {
    const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

    jerry_size_t bytes = jerry_string_to_utf8_char_buffer (jvalue->value(), (jerry_char_t *)buffer, length);

    buffer[bytes] = '\0';
    return (int)bytes;
}

int String::Length() const {
    return reinterpret_cast<const JerryValue*>(this)->GetStringLength();
}

int String::Utf8Length() const {
    return reinterpret_cast<const JerryValue*>(this)->GetStringUtf8Length();
}

String::Utf8Value::Utf8Value(Local<v8::Value> obj) : Utf8Value(Isolate::GetCurrent(), obj) { }

String::Utf8Value::Utf8Value(Isolate* isolate, Local<v8::Value> v8Value)
    : str_(nullptr)
    , length_(0)
{
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(*v8Value);

    jerry_value_t value;
    if (!jvalue->IsString()) {
        value = jerry_value_to_string(jvalue->value());
    } else {
        value = jvalue->value();
    }

    length_ = jerry_get_utf8_string_length(value);
    uint32_t size = (uint32_t)jerry_get_utf8_string_size(value);

    str_ = new char[size + 1];

    jerry_string_to_utf8_char_buffer (value, (jerry_char_t *)str_, size + 1);
    str_[size] = '\0';

    if (!jvalue->IsString()) {
        jerry_release_value(value);
    }
}

String::Utf8Value::~Utf8Value() {
    delete [] str_;
}

String::Value::~Value() {
    delete [] str_;
}

Local<String> String::Concat(Local<String> left, Local<String> right) {
    JerryValue* lhs = reinterpret_cast<JerryValue*>(*left);
    JerryValue* rhs = reinterpret_cast<JerryValue*>(*right);

    jerry_size_t lsize = jerry_get_string_size (lhs->value());
    jerry_size_t rsize = jerry_get_string_size (rhs->value());

    std::vector<char> buffer;
    buffer.resize(lsize + rsize);

    jerry_string_to_char_buffer (lhs->value(), reinterpret_cast<jerry_char_t*>(&buffer[0]), lsize);
    jerry_string_to_char_buffer (rhs->value(), reinterpret_cast<jerry_char_t*>(&buffer[0]) + lsize, rsize);

    jerry_value_t value = jerry_create_string_sz (reinterpret_cast<const jerry_char_t*>(&buffer[0]), lsize + rsize);

    RETURN_HANDLE(String, Isolate::GetCurrent(), new JerryValue(value));
}

/* Script */
MaybeLocal<Script> Script::Compile(Local<Context> context, Local<String> source, ScriptOrigin* origin /* = nullptr */) {
    jerry_char_t* sourceString = new jerry_char_t[source->Utf8Length() + 1];
    source->WriteUtf8((char*)sourceString, source->Utf8Length(), 0, 0);

    jerry_value_t scriptFunction = jerry_parse(NULL, 0, sourceString, source->Utf8Length(), JERRY_PARSE_NO_OPTS);

    delete [] sourceString;

    RETURN_HANDLE(Script, context->GetIsolate(), new JerryValue(scriptFunction));
}

MaybeLocal<Value> Script::Run(v8::Local<v8::Context> context) {
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(this);

    jerry_value_t result = jerry_run(jvalue->value());

    RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(result));
}

Local<Value> Script::Run() {
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(this);

    jerry_value_t result = jerry_run(jvalue->value());
    // TODO: report error for try-catch.

    RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(result));
}

/* Function */
MaybeLocal<Function> Function::New(Local<Context> context,
                                   FunctionCallback callback,
                                   Local<Value> data, /* = Local<Value>() */
                                   int length, /* = 0 */
                                   ConstructorBehavior behavior /* = ConstructorBehavior::kAllow */) {
    // TODO: maybe don't use function template?
    Local<FunctionTemplate> tmplt = FunctionTemplate::New(context->GetIsolate(), callback, data, Local<Signature>(), length, behavior);
    return tmplt->GetFunction();
}

Local<Object> Function::NewInstance(int argc, Local<Value> argv[]) const {
    const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

    std::vector<jerry_value_t> arguments;
    arguments.resize(argc);
    for (int idx = 0; idx < argc; idx++) {
        JerryValue* arg = reinterpret_cast<JerryValue*>(*argv[idx]);
        arguments[idx] = arg->value();
    }

    JerryValue* object = new JerryValue(jerry_construct_object(jvalue->value(), &arguments[0], (jerry_size_t)argc));
    RETURN_HANDLE(Object, Isolate::GetCurrent(), object);
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context, int argc, Local<Value> argv[]) const {
    return NewInstance(argc, argv);
}

Local<Value> Function::Call(Local<Value> recv, int argc, Local<Value> argv[]) {
    const JerryValue* jfunc = reinterpret_cast<const JerryValue*>(this);
    const JerryValue* jthis = reinterpret_cast<const JerryValue*>(*recv);

    std::vector<jerry_value_t> arguments;
    arguments.resize(argc);
    for (int idx = 0; idx < argc; idx++) {
        arguments[idx] = reinterpret_cast<JerryValue*>(*argv[idx])->value();
    }

    jerry_value_t result = jerry_call_function(jfunc->value(), jthis->value(), &arguments[0], argc);
    RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(result));
}

MaybeLocal<Value> Function::Call(Local<Context> context, Local<Value> recv, int argc, Local<Value> argv[]) {
    return Call(recv, argc, argv);
}

void Function::SetName(Local<String> name) {
    // TODO: how to set the "name" of a function (in JS the function.name is readonly)
}

Local<Value> Function::GetDebugName() const {
    // TODO: only used by node_perf.cc
    return Local<Value>();
}

Local<Value> Function::GetBoundFunction() const {
    // TODO: only used by node_perf.cc
    return Local<Value>();
}


/* Function Template */
void FunctionTemplate::SetCallHandler(FunctionCallback callback,
                                      v8::Local<Value> data) {
    JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
    func->SetCallback(callback);
    if (!data.IsEmpty()) {
        JerryValue* value = reinterpret_cast<JerryValue*>(*data);
        jerry_value_t jvalue = jerry_acquire_value(value->value());

        func->SetExternalData(jvalue);
    }
}

Local<FunctionTemplate> FunctionTemplate::New(Isolate* isolate,
                                              FunctionCallback callback /*= 0*/,
                                              Local<Value> data /*= Local<Value>()*/,
                                              Local<Signature> signature /* = Local<Signature>() */,
                                              int length /* = 0 */,
                                              ConstructorBehavior behavior /* = ConstructorBehavior::kAllow */) {
    // TODO: handle the other args
    JerryFunctionTemplate* func = new JerryFunctionTemplate();
    reinterpret_cast<FunctionTemplate*>(func)->SetCallHandler(callback, data);

    RETURN_HANDLE(FunctionTemplate, isolate, func);
}

Local<Function> FunctionTemplate::GetFunction() {
    JerryFunctionTemplate* tmplt = reinterpret_cast<JerryFunctionTemplate*>(this);
    JerryValue *func = tmplt->GetFunction()->Copy();

    RETURN_HANDLE(Function, Isolate::GetCurrent(), func);
}

void FunctionTemplate::SetClassName(Local<String> name) {
    // TODO: This should be used as the constructor's name. Skip this for now.
}

bool FunctionTemplate::HasInstance(Local<Value> object) {
    JerryValue* value = reinterpret_cast<JerryValue*>(*object);

    if (!value->IsObject() || value->IsFunction()) {
        return false;
    }

    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(value->value(), &native_p, &JerryV8ObjectConstructed);

    if (!has_data) {
        return false;
    }

    JerryV8FunctionHandlerData* data = reinterpret_cast<JerryV8FunctionHandlerData*>(native_p);

    // TODO: the prototype chain should be traversed

    // TODO: do a better check not just a simple address check
    return data->function_template == reinterpret_cast<JerryFunctionTemplate*>(this);
}

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
    JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
    JerryObjectTemplate* obj_template = func->PrototypeTemplate();
    RETURN_HANDLE(ObjectTemplate, Isolate::GetCurrent(), obj_template);
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
    JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
    JerryObjectTemplate* obj_template = func->InstanceTemplate();
    RETURN_HANDLE(ObjectTemplate, Isolate::GetCurrent(), obj_template);
}

/*  Template */
void Template::Set(v8::Local<v8::Name> name, v8::Local<v8::Data> data, v8::PropertyAttribute attributes) {
    JerryTemplate* templt = reinterpret_cast<JerryTemplate*>(this);

    // Here we copy the values as the template's internl elements will do a release on them
    JerryValue* key = reinterpret_cast<JerryValue*>(*name)->Copy();
    // TODO: maybe this should not be a JerryValue?
    JerryValue* value = reinterpret_cast<JerryValue*>(*data)->Copy();

    templt->Set(key, value, attributes);
}

void Template::SetAccessorProperty(
    Local<Name> name,
    Local<FunctionTemplate> getter /* = Local<FunctionTemplate>()*/,
    Local<FunctionTemplate> setter /* = Local<FunctionTemplate>()*/,
    PropertyAttribute attribute /* = None */,
    AccessControl settings /*= DEFAULT */) {

    JerryValue* key = reinterpret_cast<JerryValue*>(*name)->Copy();
    // TODO: maybe this should not be a JerryValue?

    JerryValue* jgetter = NULL;
    JerryValue* jsetter = NULL;
    if (!getter.IsEmpty()) {

        jgetter = reinterpret_cast<JerryFunctionTemplate*>(*getter)->GetFunction()->Copy();
    }

    if (!setter.IsEmpty()) {
        jgetter = reinterpret_cast<JerryFunctionTemplate*>(*setter)->GetFunction()->Copy();
    }
    // TODO: handle attributes and settings

    JerryTemplate* templt = reinterpret_cast<JerryTemplate*>(this);
    templt->SetAccessorProperty(key, jgetter, jsetter, attribute);
}

/* ObjectTemplate */
Local<ObjectTemplate> ObjectTemplate::New(Isolate* isolate, Local<FunctionTemplate> constructor /* = Local<FunctionTemplate>() */) {
    JerryObjectTemplate* obj_template = new JerryObjectTemplate();

    if (!constructor.IsEmpty()) {
        // TODO: check if this is correct
        JerryFunctionTemplate* function_template = reinterpret_cast<JerryFunctionTemplate*>(*constructor);
        obj_template->SetConstructor(function_template);
    }

    RETURN_HANDLE(ObjectTemplate, isolate, obj_template);
}

MaybeLocal<Object> ObjectTemplate::NewInstance(Local<Context> context) {
    JerryObjectTemplate* object_template = reinterpret_cast<JerryObjectTemplate*>(this);

    // TODO: the function template's method should be set as the object's constructor
    JerryValue* new_instance = new JerryValue(jerry_create_object());
    object_template->InstallProperties(new_instance->value());

    RETURN_HANDLE(Object, context->GetIsolate(), new_instance);
}

void ObjectTemplate::SetHandler(v8::NamedPropertyHandlerConfiguration const& handler) {
    // TODO: JerryScript: Add support for "interceptors"
}

/** SetAccessor: { value: 1556,
  writable: true,
  enumerable: true,
  configurable: true }
*/
void ObjectTemplate::SetAccessor(Local<String> name,
                                 AccessorGetterCallback getter,
                                 AccessorSetterCallback setter /* = 0 */,
                                 Local<Value> data /* = Local<Value>() */,
                                 AccessControl settings /* = DEFAULT */,
                                 PropertyAttribute attribute /* = None */,
                                 Local<AccessorSignature> signature /* = Local<AccessorSignature>() */) {
    JerryObjectTemplate* tmplt = reinterpret_cast<JerryObjectTemplate*>(this);

    JerryValue* jname = reinterpret_cast<JerryValue*>(*name)->Copy();
    JerryValue* jdata = NULL;
    if (!data.IsEmpty()) {
        jdata = reinterpret_cast<JerryValue*>(*data)->Copy();
    }

    tmplt->SetAccessor(jname, getter, setter, jdata, settings, attribute);
}

void ObjectTemplate::SetInternalFieldCount(int count) {
    JerryObjectTemplate* tmplt = reinterpret_cast<JerryObjectTemplate*>(this);
    tmplt->SetInteralFieldCount(count);
}

/* Signature */
Local<Signature> Signature::New(Isolate* isolate, Local<FunctionTemplate> receiver) {
    return Local<Signature>(reinterpret_cast<Signature*>(*receiver));
}

/* Exception & Error */
#define EXCEPTION_ERROR(error_class, error_type) \
    Local<Value> Exception::error_class(Local<String> message) { \
        JerryValue* jstr = reinterpret_cast<JerryValue*>(*message); \
        jerry_size_t req_sz = jerry_get_utf8_string_size(jstr->value()); \
        jerry_char_t str_buf_p[req_sz]; \
        jerry_string_to_utf8_char_buffer(jstr->value(), str_buf_p, req_sz); \
        jerry_value_t error_obj = jerry_create_error_sz (error_type, str_buf_p, req_sz); \
        RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(error_obj)); \
    }

EXCEPTION_ERROR(Error, JERRY_ERROR_COMMON);
EXCEPTION_ERROR(RangeError, JERRY_ERROR_RANGE);
EXCEPTION_ERROR(TypeError, JERRY_ERROR_TYPE);

/* StackFrame && StackTrace */
int StackFrame::GetColumn() const {
    return 5;
}

int StackFrame::GetScriptId() const {
    return 0;
}

bool StackFrame::IsEval() const {
    return true;
}

int StackTrace::GetFrameCount() const {
    return 0;
}

Local<String> StackFrame::GetFunctionName() const {
    return Local<String>();
}

int StackFrame::GetLineNumber() const {
    return 0;
}

Local<String> StackFrame::GetScriptName() const {
    return Local<String>();
}

Local<StackTrace> StackTrace::CurrentStackTrace(v8::Isolate*, int, StackTrace::StackTraceOptions) {
    return Local<StackTrace>();
}

Local<StackFrame> StackTrace::GetFrame(unsigned int) const {
    return Local<StackFrame>();
}

/* HeapProfiler & HeapStatistics */
void HeapProfiler::SetWrapperClassInfoProvider(unsigned short class_id, WrapperInfoCallback cb) { }
void HeapProfiler::StartTrackingHeapObjects(bool track_allocations) { }

v8::HeapStatistics::HeapStatistics() { }

/* Dummy tracing */
namespace platform {
namespace tracing {

TraceBufferChunk::TraceBufferChunk(uint32_t seq) { }

TraceObject* TraceBufferChunk::AddTraceEvent(size_t* event_index) {
    return NULL;
}

void TraceBufferChunk::Reset(uint32_t new_seq) { }


void TraceConfig::AddIncludedCategory(char const* included_category) { }

TraceObject::~TraceObject() { }

TraceWriter* TraceWriter::CreateJSONTraceWriter(std::ostream& stream) {
    return NULL;
}

TracingController::TracingController() { }
TracingController::~TracingController() { }
void TracingController::Initialize(TraceBuffer* trace_buffer) { }
void TracingController::StartTracing(TraceConfig* trace_config) { }
void TracingController::StopTracing() { }
const uint8_t* TracingController::GetCategoryGroupEnabled(const char* category_group) {
    return NULL;
}

uint64_t TracingController::AddTraceEvent(
        char phase, const uint8_t* category_enabled_flag, const char* name,
        const char* scope, uint64_t id, uint64_t bind_id, int32_t num_args,
        const char** arg_names, const uint8_t* arg_types,
        const uint64_t* arg_values,
        std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
        unsigned int flags) {
    return 0;
}

void TracingController::UpdateTraceEventDuration(const uint8_t* category_enabled_flag, const char* name, uint64_t handle) { }
void TracingController::AddTraceStateObserver(v8::TracingController::TraceStateObserver* observer) { }
void TracingController::RemoveTraceStateObserver(v8::TracingController::TraceStateObserver* observer) { }

} // namespace tracing
} // namespace platform
/* Dummy tracing END*/

/* DebugBreak */
void Debug::DebugBreak(Isolate*) {
    // TODO: add trace
}

} // namespace v8
