#ifndef V8JERRY_VALUE_HPP
#define V8JERRY_VALUE_HPP

#include <string>
#include <vector>

#include "v8.h"
#include "v8jerry_utils.hpp"

class JerryIsolate;
struct JerryV8ContextData;

class JerryHandle {
public:
    enum Type {
        Context,
        FunctionTemplate,
        ObjectTemplate,
        Value,

        GlobalValue,
    };

    JerryHandle() {}

    JerryHandle(Type type)
        : m_type(type)
    {}

    Type type() const { return m_type; }


    static bool IsValueType(JerryHandle* handle) {
        return (handle != NULL) &&
                ((handle->type() == Value) || (handle->type() == GlobalValue));
    }

private:
    Type m_type;
};

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

struct JerryV8WeakReferenceData {
    v8::WeakCallbackInfo<void>::Callback callback;
    v8::WeakCallbackType type;
    void* data;

    JerryV8WeakReferenceData(v8::WeakCallbackInfo<void>::Callback callback, v8::WeakCallbackType type, void* data)
        : callback(callback)
        , type(type)
        , data(data)
        {
        }
};

class JerryValue : public JerryHandle {
public:
    JerryValue()
        : JerryValue(0, false)
    {}

    JerryValue(jerry_value_t value)
        : JerryValue(value, false)
    {}

    /* Create a JerryValue if there is no error.
     * If the "value" is an error report it to the Isolate (for try-catch)
     */
    static JerryValue* TryCreateValue(JerryIsolate* iso, jerry_value_t value);

    ~JerryValue(void) {
        if (m_value) {
            jerry_release_value(m_value);
            m_value = 0;
        }
    }

    jerry_value_t value() const { return m_value; }

    bool SetProperty(JerryValue* key, JerryValue* value);
    bool SetPropertyIdx(uint32_t idx, JerryValue* value);

    JerryValue* GetProperty(JerryValue* key);
    JerryValue* GetPropertyIdx(uint32_t idx);

    JerryValue* GetOwnPropertyDescriptor(const JerryValue& jkey) const;

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
    bool IsNull() const { return jerry_value_is_null(m_value); }
    bool IsUndefined() const { return jerry_value_is_undefined(m_value); }

    double GetNumberValue(void) const { return jerry_get_number_value(m_value); }
    uint32_t GetUInt32Value(void) const { return (uint32_t)jerry_get_number_value(m_value); }
    int32_t GetInt32Value(void) const { return (int32_t)jerry_get_number_value(m_value); }
    int64_t GetInt64Value(void) const { return (int64_t)jerry_get_number_value(m_value); }
    bool GetBooleanValue(void) const { return jerry_get_boolean_value(m_value); }

    int GetStringLength(void) const { return jerry_get_string_length(m_value); }
    int GetStringUtf8Length(void) const { return jerry_get_utf8_string_size(m_value); }

    JerryValue* ToString(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_string(m_value));
    }

    JerryValue* ToInteger(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_number(m_value));
    }

    bool ToBoolean(void) const {
        // TODO: error handling?
        return jerry_value_to_boolean(m_value);
    }

    JerryValue* ToObject(void) const {
        // TODO: error handling?
        return new JerryValue(jerry_value_to_object(m_value));
    }

    JerryValue* GetObjectCreationContext(void);

    static JerryValue* NewPromise(void);
    static JerryValue* NewObject(void);
    static JerryValue* NewExternal(void* ptr);

    static JerryValue* NewContextObject(JerryIsolate* iso);
    bool IsContextObject(void);
    JerryV8ContextData* ContextGetData(void);
    JerryIsolate* ContextGetIsolate(void);
    void ContextEnter(void);
    void ContextExit(void);
    void ContextSetEmbedderData(int index, void* value);
    void* ContextGetEmbedderData(int index);


    void* GetExternalData(void) const;
    bool IsExternal() const;

    JerryValue* Copy() const { return new JerryValue(jerry_acquire_value(m_value)); }
    JerryValue* CopyToGlobal() const { return new JerryValue(jerry_acquire_value(m_value), true); }

    void MakeWeak(v8::WeakCallbackInfo<void>::Callback weak_callback, v8::WeakCallbackType type, void* data);
    bool IsWeakReferenced();
    void* ClearWeak();
    void RunWeakCleanup();


    static void CreateInternalFields(jerry_value_t target, int field_count);
    JerryV8InternalFieldData* GetInternalFieldData(int idx);
    void SetInternalField(int idx, JerryValue* value);
    void SetInternalField(int idx, void* value);

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

    int InternalFieldCount(void);

    JerryValue(JerryValue& that) = delete;

    template <typename T>
    v8::Local<T> AsLocal(void) {
        /* Magical dragons from the depths are present in this method!
         * You have been warned! Proceed with caution!
         */
        JerryValue* value_ptr = this;
        v8::Local<T>* v8_value = reinterpret_cast<v8::Local<T>*>(&value_ptr);
        return *v8_value;
    }

private:
    JerryValue(jerry_value_t value, bool isGlobal)
        : JerryHandle(isGlobal ? JerryHandle::GlobalValue : JerryHandle::Value)
        , m_value(value)
    {}

    jerry_value_t m_value;
};

#endif /* V8JERRY_VALUE_HPP */
