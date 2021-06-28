#ifndef V8JERRY_VALUE_HPP
#define V8JERRY_VALUE_HPP

#include <string>
#include <vector>

#include "v8.h"
#include "v8jerry_backing_store.hpp"
#include "v8jerry_utils.hpp"

#include "jerryscript.h"

class JerryIsolate;
class JerryHandle;
struct JerryV8ContextData;

namespace v8 {
    class Utils {
    public:
        template <typename T> static
        v8::Local<T> AsLocal(JerryHandle* ptr) {
            return v8::Local<T>(reinterpret_cast<T*>(ptr));
        }

        template <typename T> static
        v8::Local<T> NewLocal(v8::Isolate* isolate, JerryHandle* ptr) {
            return v8::Local<T>::New(isolate, reinterpret_cast<T*>(ptr));
        }

        static inline CompiledWasmModule Convert(
           std::shared_ptr<internal::wasm::NativeModule> native_module) {
           return CompiledWasmModule{std::move(native_module)};
        }
    };
}

static void JerryV8WeakCallback(void* native_p, jerry_object_native_info_t* info_p);

class JerryHandle {
    friend void JerryV8WeakCallback(void* native_p, jerry_object_native_info_t* info_p);
public:
    enum Type {
        Context,
        FunctionTemplate,
        ObjectTemplate,

        // Only value types are allowed after this point
        LocalValue,
        PersistentValue,
        PersistentWeakValue,
        PersistentDeletedValue,
    };

    JerryHandle() {}

    JerryHandle(Type type)
        : m_type(static_cast<uint8_t>(type))
        , m_stringType(0)
    {}

    Type type() const { return static_cast<Type>(m_type); }

    static bool IsValueType(JerryHandle* handle) {
        return (handle != NULL && handle->type() >= LocalValue);
    }

    template <typename T>
    v8::Local<T> AsLocal(void) {
        return v8::Utils::AsLocal<T>(this);
    }

    template <typename T>
    v8::Local<T> NewLocal(v8::Isolate* isolate) {
        return v8::Utils::NewLocal<T>(isolate, this);
    }

protected:
    uint8_t m_type;
    uint8_t m_stringType;
};

struct JerryV8InternalFieldData {
    // TODO: maybe use raw pointers to reduce memory?
    int count;
    std::vector<void*> fields;

    JerryV8InternalFieldData(int count)
        : count(count)
        , fields(count, NULL)
    {
    }
};

struct JerryV8WeakReference {
    JerryV8WeakReference *next;
    JerryHandle *persistent;
    v8::WeakCallbackInfo<void>::Callback callback;
    v8::WeakCallbackType type;
    void* data;

    JerryV8WeakReference(JerryV8WeakReference *next, JerryHandle *persistent, v8::WeakCallbackInfo<void>::Callback callback,
                         v8::WeakCallbackType type, void* data)
        : next(next)
        , persistent(persistent)
        , callback(callback)
        , type(type)
        , data(data)
        {
        }
};

class JerryString;
class JerryValue;

class JerryValueNoRelease : public JerryHandle {
public:
    JerryValueNoRelease(jerry_value_t value)
        : JerryHandle(JerryHandle::LocalValue)
        , m_value(value)
    {}

    jerry_value_t value() const { return m_value; }

protected:
    JerryValueNoRelease(jerry_value_t value, JerryHandle::Type type)
        : JerryHandle(type)
        , m_value(value)
    {}

    jerry_value_t m_value;
};

class JerryValue : public JerryValueNoRelease {
public:
    JerryValue(jerry_value_t value, JerryHandle::Type type)
        : JerryValueNoRelease(value, type)
    {}

    JerryValue()
        : JerryValueNoRelease(jerry_create_undefined(), JerryHandle::LocalValue)
    {}

    JerryValue(jerry_value_t value)
        : JerryValueNoRelease(value, JerryHandle::LocalValue)
    {}

    /* Create a JerryValue if there is no error.
     * If the "value" is an error report it to the Isolate (for try-catch)
     */
    static JerryValue* TryCreateValue(v8::Isolate* isolate, jerry_value_t value);

    /* Deletes the value without releasing the internal Jerry value. */
    static void DeleteValueWithoutRelease(JerryValue *value) {
        delete reinterpret_cast<JerryValueNoRelease*>(value);
    }

    ~JerryValue(void) {
        jerry_release_value(m_value);
    }

    JerryValue *clone() const { return new JerryValue(jerry_acquire_value(m_value)); }

    bool SetProperty(JerryValue* key, JerryValue* value);
    bool SetPrivateProperty(JerryValue* key, JerryValue* value);


    bool SetInternalProperty(JerryValue* key, JerryValue* value);
    bool HasInternalProperty(JerryValue* key);
    bool DeleteInternalProperty(JerryValue* key);

    JerryValue* GetOwnPropertyDescriptor(const JerryValue& jkey) const;
    JerryValue* GetOwnPropertyNames() const;

    bool IsString() const { return jerry_value_is_string(m_value); }
    bool IsName() const { return jerry_value_is_string(m_value) || jerry_value_is_symbol(m_value); }
    bool IsBoolean() const { return jerry_value_is_boolean(m_value); }
    bool IsFalse() const { return jerry_value_is_boolean(m_value) && !jerry_value_is_true(m_value); }
    bool IsTrue() const { return jerry_value_is_boolean(m_value) && jerry_value_is_true(m_value); }
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
    bool IsProxy() const { return jerry_value_is_proxy (m_value); }
    bool IsRegExp() const { return jerry_object_get_type(m_value) == JERRY_OBJECT_TYPE_REGEXP; }
    bool IsSharedArrayBuffer() const { return false; }
    bool IsNativeError() const { return jerry_get_error_type(m_value) != JERRY_ERROR_NONE; }
    bool IsModuleNameSpaceObject() const { return jerry_object_get_type(m_value) == JERRY_OBJECT_TYPE_MODULE_NAMESPACE; }
    bool IsBigInt() const { return jerry_value_is_bigint(m_value); }
    bool IsArrayBufferView() const { return jerry_value_is_typedarray(m_value) || jerry_value_is_dataview(m_value); }
    bool IsDataView() const { return jerry_value_is_dataview(m_value); }
    bool IsNull() const { return jerry_value_is_null(m_value); }
    bool IsUndefined() const { return jerry_value_is_undefined(m_value); }

    double GetNumberValue(void) const { return jerry_get_number_value(m_value); }
    uint32_t GetUInt32Value(void) const { return jerry_value_as_uint32(m_value); }
    int32_t GetInt32Value(void) const { return jerry_value_as_int32(m_value); }
    int64_t GetInt64Value(void) const { return jerry_value_as_integer(m_value); }
    bool GetBooleanValue(void) const { return jerry_value_is_true(m_value); }

    int GetStringLength(void) const { return jerry_get_string_length(m_value); }
    int GetStringUtf8Length(void) const { return jerry_get_utf8_string_size(m_value); }

    JerryValue* GetProperty(v8::Isolate* isolate, JerryValue* key) const { return TryCreateValue(isolate, jerry_get_property(m_value, key->value())); }
    JerryValue* GetPropertyIdx(v8::Isolate* isolate, uint32_t idx) const { return TryCreateValue(isolate, jerry_get_property_by_index(m_value, idx)); }
    JerryValue* GetPrivateProperty(v8::Isolate* isolate, JerryValue* key) const { return TryCreateValue(isolate, jerry_get_internal_property(m_value, key->value())); }

    JerryValue* ToString(v8::Isolate* isolate) const { return TryCreateValue(isolate, jerry_value_to_string(m_value)); }
    JerryValue* ToNumber(v8::Isolate* isolate) const { return TryCreateValue(isolate, jerry_value_to_number(m_value)); }
    JerryValue* ToObject(v8::Isolate* isolate) const { return TryCreateValue(isolate, jerry_value_to_object(m_value)); }
    JerryValue* ToInteger(v8::Isolate* isolate) const;

    bool BooleanValue() const {
        return jerry_value_is_true(m_value);
    }

    JerryValue* ToBoolean(void) const {
        return new JerryValue(jerry_value_to_boolean(m_value));
    }


    jerry_value_t GetObjectCreationContext(void);

    static JerryValue* NewPromise(void);
    static JerryValue* NewObject(void);
    static JerryValue* NewArrayBuffer(JerryBackingStore *backingStore);
    static JerryValue* NewTypedArray(JerryValue* array_buffer,
                                     size_t byte_offset, size_t length, jerry_typedarray_type_t type);
    static JerryValue* NewExternal(void* ptr);

    static JerryValue* NewContextObject(JerryIsolate* iso);
    bool IsContextObject(void);
    JerryV8ContextData* ContextGetData(void);
    JerryIsolate* ContextGetIsolate(void);
    void ContextEnter(void);
    void ContextExit(void);
    void ContextSetEmbedderData(int index, void* value);
    void* ContextGetEmbedderData(int index);

    JerryBackingStore* GetBackingStore(void) const;
    void* GetExternalData(void) const;
    bool IsExternal() const;

    JerryValue* Move(JerryValue* to) { jerry_release_value(m_value); m_value = jerry_acquire_value(to->value()); }
    JerryValue* Copy() const { return new JerryValue(jerry_acquire_value(m_value)); }
    JerryValue* CopyToGlobal() const { return new JerryValue(jerry_acquire_value(m_value), JerryHandle::PersistentValue); }

    void MakeWeak(v8::WeakCallbackInfo<void>::Callback weak_callback, v8::WeakCallbackType type, void* data);
    void* ClearWeak();

    static void CreateInternalFields(jerry_value_t target, int field_count);
    JerryV8InternalFieldData* GetInternalFieldData(int idx);
    void SetInternalField(int idx, JerryValue* value);
    void SetInternalField(int idx, void* value);
    int InternalFieldCount(void);

    JerryValue* GetInternalJerryValue(int idx) {
        JerryV8InternalFieldData* data = GetInternalFieldData(idx);
        if (data == NULL) {
            return new JerryValue(jerry_create_undefined());
        }

        uintptr_t value = reinterpret_cast<uintptr_t>(data->fields[idx]);
        return new JerryValue(jerry_acquire_value(static_cast<jerry_value_t>(value)));
    }

    void* GetInternalPointer(int idx) {
        JerryV8InternalFieldData* data = GetInternalFieldData(idx);
        if (data == NULL) {
            return NULL;
        }

        return data->fields[idx];
    }

    JerryValue(JerryValue& that) = delete;
};

enum JerryStringType {
    ONE_BYTE = 0x0,
    TWO_BYTE = 0x1,
    EXTERNAL = 0x2,
};

class JerryString : public JerryValue {
public:
    JerryString(jerry_value_t value, JerryStringType type = JerryStringType::ONE_BYTE)
        : JerryValue(value)
    {
        m_stringType = static_cast<uint8_t>(type);
    }

    JerryStringType type () const {
        return static_cast<JerryStringType>(m_stringType);
    }
};

class JerryExternalString : public JerryString {
public:
    JerryExternalString(jerry_value_t value, v8::String::ExternalStringResourceBase* resource, JerryStringType type = JerryStringType::ONE_BYTE)
        : JerryString(value, (JerryStringType) (JerryStringType::EXTERNAL | type))
        , m_resource(resource)
    {}

    v8::String::ExternalStringResourceBase* resource () const {
        return m_resource;
    }

    ~JerryExternalString();

private:
    v8::String::ExternalStringResourceBase* m_resource;
};


#endif /* V8JERRY_VALUE_HPP */
