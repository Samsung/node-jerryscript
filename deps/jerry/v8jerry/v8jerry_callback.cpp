#include "v8jerry_callback.hpp"

#include <v8.h>

#include "v8jerry_value.hpp"
#include "v8jerry_templates.hpp"
#include "v8jerry_isolate.hpp"

#define DEFINE_RETURN_VALUE() \
    v8::ReturnValue<v8::Value> returnValueSlot = info.GetReturnValue(); \
    JerryValue** returnValue = *reinterpret_cast<JerryValue***>(&returnValueSlot); \
    JerryValue** savedReturnValue = isolate->GetLastReturnValue(); \
    isolate->SetLastReturnValue(returnValue);

#define RESTORE_RETURN_VALUE() \
    isolate->SetLastReturnValue(savedReturnValue);

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
        const jerry_value_t external_data)
        : m_this_value(this_val)
        , m_external_data_value(external_data)
        , v8::PropertyCallbackInfo<T>(reinterpret_cast<v8::internal::Address*>(m_implicit_args))
    {
        BuildArgs(this_val, external_data);
    }

private:
    void BuildArgs(const jerry_value_t this_val, const jerry_value_t external_data) {
        JerryIsolate *isolate = JerryIsolate::fromV8(v8::Isolate::GetCurrent());

        m_implicit_args[v8::PropertyCallbackInfo<T>::kShouldThrowOnErrorIndex] = (JerryHandle*)v8::internal::IntToSmi(v8::internal::Internals::kDontThrow); // TODO: fix this
        m_implicit_args[v8::PropertyCallbackInfo<T>::kHolderIndex] = &m_this_value; // TODO: 'this' object is not correct
        // TODO: correctly fill the arguments:
        m_implicit_args[v8::PropertyCallbackInfo<T>::kIsolateIndex] = reinterpret_cast<JerryHandle*>(isolate);
        //values[v8::PropertyCallbackInfo<T>::kReturnValueDefaultValueIndex] = ; // TODO
        m_implicit_args[v8::PropertyCallbackInfo<T>::kReturnValueIndex] = isolate->Hole();

        m_implicit_args[v8::PropertyCallbackInfo<T>::kDataIndex] = &m_external_data_value;
        m_implicit_args[v8::PropertyCallbackInfo<T>::kThisIndex] = &m_this_value;
    }

    JerryHandle* m_implicit_args[kImplicitArgsSize];
    JerryValueNoRelease m_this_value;
    JerryValueNoRelease m_external_data_value;
};

jerry_value_t JerryV8GetterSetterHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt) {
    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(function_obj, &native_p, &JerryV8GetterSetterHandlerData::TypeInfo);

    if (!has_data) {
        fprintf(stderr, "ERRR.... should not be here!(%s:%d)\n", __FILE__, __LINE__);
        abort();
        return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t*)"BAD!!");
    }
    JerryV8GetterSetterHandlerData* data = reinterpret_cast<JerryV8GetterSetterHandlerData*>(native_p);

    // Make sure that Localy allocated vars will be freed upon exit.
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

    jerry_value_t jret = jerry_create_undefined();

    v8::Local<v8::Name> v8_name = data->accessor->name->AsLocal<v8::Name>();

    if (data->is_setter) {
        JerryPropertyCallbackInfo<void> info(function_obj, this_val, args_p, args_cnt, data->external);

        // TODO: assert on args[0]?
        JerryValueNoRelease new_value(args_p[0]);

        v8::Local<v8::Value> v8_value = new_value.AsLocal<v8::Value>();

        if (data->is_named) {
            data->v8.setter.named(v8_name, v8_value, info);
        } else {
            data->v8.setter.stringed(v8_name.As<v8::String>(), v8_value, info);
        }
    } else {
        JerryIsolate* isolate = JerryIsolate::GetCurrent();
        JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, data->external);

        DEFINE_RETURN_VALUE();
        if (data->is_named) {
            data->v8.getter.named(v8_name, info);
        } else {
            data->v8.getter.stringed(v8_name.As<v8::String>(), info);
        }
        RESTORE_RETURN_VALUE();

        if (!isolate->HasError()) {
            jret = jerry_acquire_value((*returnValue)->value());
        } else {
            jret = jerry_create_error_from_value(isolate->TakeError(), true);
        }
    }

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
        const jerry_value_t new_target_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt,
        const JerryV8FunctionHandlerData* handlerData)
        : m_this_value(this_val)
        , m_new_target_value(new_target_val)
        , m_external_data_value(handlerData->function_template->external())
        , v8::FunctionCallbackInfo<T>(reinterpret_cast<v8::internal::Address*>(m_implicit_args),
                                      reinterpret_cast<v8::internal::Address*>(BuildArgs(args_p, args_cnt)),
                                      args_cnt)
    {
        BuildImplicitArgs();
    }

    ~JerryFunctionCallbackInfo() {
        /* The values_ points to the end */
        JerryValue **values = reinterpret_cast<JerryValue**>(this->values_) - this->Length() + 1;

        for (int i = 0; i < this->Length(); i++) {
            JerryValue::DeleteValueWithoutRelease(values[i]);
        }

        delete[] values;
    }

private:
    static JerryValue* newValue(jerry_value_t value) {
        /* Handle scope will release these values */
        JerryValue* jvalue = new JerryValue(jerry_acquire_value(value));
        JerryIsolate::GetCurrent()->AddToHandleScope(jvalue);
        return jvalue;
    }

    JerryHandle** BuildArgs(const jerry_value_t args_p[], const jerry_length_t args_cnt) {

        JerryHandle **values = new JerryHandle*[args_cnt + 1 /* for this */];
        JerryIsolate *isolate = JerryIsolate::GetCurrent();

        /* args_p[0]  is at 'values - 0' */
        /* args_p[1]  is at 'values - 1' */
        /* args_p[2]  is at 'values - 2' */
        for (jerry_length_t idx = 0; idx < args_cnt; idx++) {
            values[args_cnt - idx - 1] = new JerryValue(args_p[idx]);
        }

        /* this should be at 'values + 1' */
        values[args_cnt] = &m_this_value;

        return values + args_cnt - 1;
    }

    void BuildImplicitArgs() {
        /* TODO: From the docs:
         * If the callback was created without a Signature, this is the same
         * value as This(). If there is a signature, and the signature didn't match
         * This() but one of its hidden prototypes, this will be the respective
         * hidden prototype.
         */
        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        JerryIsolate* jerry_isolate = reinterpret_cast<JerryIsolate*>(isolate);

        m_implicit_args[v8::FunctionCallbackInfo<T>::kHolderIndex] = &m_this_value;
        // TODO: correctly fill the arguments:
        m_implicit_args[v8::FunctionCallbackInfo<T>::kIsolateIndex] = reinterpret_cast<JerryHandle*>(isolate);
        m_implicit_args[v8::FunctionCallbackInfo<T>::kReturnValueIndex] = jerry_isolate->Undefined();

        m_implicit_args[v8::FunctionCallbackInfo<T>::kDataIndex] = &m_external_data_value;
        m_implicit_args[v8::FunctionCallbackInfo<T>::kNewTargetIndex] = &m_new_target_value;
    }

    JerryHandle* m_implicit_args[kImplicitArgsSize];
    JerryValueNoRelease m_this_value;
    JerryValueNoRelease m_new_target_value;
    JerryValueNoRelease m_external_data_value;
};

JerryV8FunctionHandlerData* JerryGetFunctionHandlerData(jerry_value_t target) {
    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(target, &native_p, &JerryV8FunctionHandlerData::TypeInfo);

    if (!has_data) {
        return NULL;
    }

    return reinterpret_cast<JerryV8FunctionHandlerData*>(native_p);
}

jerry_value_t JerryV8FunctionHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt) {

    void *native_p;
    bool has_data = jerry_get_object_native_pointer(function_obj, &native_p, &JerryV8FunctionHandlerData::TypeInfo);

    if (!has_data) {
        fprintf(stderr, "ERRR.... should not be here!(%s:%d)\n", __FILE__, __LINE__);
        abort();
        return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t*)"BAD!!");
    }
    JerryV8FunctionHandlerData* data = reinterpret_cast<JerryV8FunctionHandlerData*>(native_p);

    // Make sure that Localy allocated vars will be freed upon exit.
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

    jerry_value_t new_target = jerry_get_new_target ();

    // If this is a constructor call do a few extra things
    if (!jerry_value_is_undefined(new_target)) {
        if (!jerry_get_object_native_pointer(this_val, NULL, &JerryV8FunctionHandlerData::TypeInfo)) {
            // Add reference to the function template
            if (data->function_template->HasInstanceTemplate()) {
                JerryObjectTemplate* tmplt = data->function_template->InstanceTemplate();
                tmplt->InstallProperties(this_val);
            }

            data->ref_count++;
            jerry_set_object_native_pointer(this_val, data, &JerryV8FunctionHandlerData::TypeInfo);
        }
    }

    if (data->function_template->HasSignature()) {
        void *native_p;
        bool has_data = jerry_get_object_native_pointer(this_val, &native_p, &JerryV8FunctionHandlerData::TypeInfo);

        if (!has_data
            || data->function_template->IsValidSignature(((JerryV8FunctionHandlerData*)native_p)->function_template->Signature())) {
            // Invalid signature found throw error.
            return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t *) "Incorrect signature");
        }
    }

    jerry_value_t jret = jerry_create_undefined();

    if (data->v8callback != NULL) {
        JerryIsolate* isolate = JerryIsolate::GetCurrent();
        JerryFunctionCallbackInfo<v8::Value> info(function_obj, this_val, new_target, args_p, args_cnt, data);

        DEFINE_RETURN_VALUE();
        data->v8callback(info);
        RESTORE_RETURN_VALUE();

        if (!isolate->HasError()) {
            jret = jerry_acquire_value((*returnValue)->value());
        } else {
            jret = jerry_create_error_from_value(isolate->TakeError(), true);
        }
    }

    jerry_release_value(new_target);
    return jret;
}

static void setPropertyDescriptorHelperBoolean(jerry_value_t object, const char *name, bool value)
{
    jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) name);
    jerry_value_t boolean_value = jerry_create_boolean (value);
    jerry_set_property (object, prop_name, boolean_value);
    jerry_release_value (boolean_value);
    jerry_release_value (prop_name);
}

static void setPropertyDescriptorHelper(jerry_value_t object, const char *name, jerry_value_t value)
{
    jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) name);
    jerry_set_property (object, prop_name, value);
    jerry_release_value (prop_name);
}

jerry_value_t JerryV8ProxyHandler(
    const jerry_value_t function_obj, const jerry_value_t this_val, const jerry_value_t args_p[], const jerry_length_t args_cnt) {

    // TODO: extract the native pointer extraction to a method
    void *native_p;
    bool has_data = jerry_get_object_native_pointer(function_obj, &native_p, &JerryV8ProxyHandlerData::TypeInfo);

    if (!has_data) {
        fprintf(stderr, "ERRR.... should not be here!(%s:%d)\n", __FILE__, __LINE__);
        abort();
        return jerry_create_error(JERRY_ERROR_REFERENCE, (const jerry_char_t*)"BAD!!");
    }
    JerryV8ProxyHandlerData* data = reinterpret_cast<JerryV8ProxyHandlerData*>(native_p);

    // Make sure that Localy allocated vars will be freed upon exit.
    v8::Isolate* v8isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(v8isolate);
    JerryIsolate* isolate = JerryIsolate::fromV8(v8isolate);

    jerry_value_t jret = jerry_create_undefined();
    jerry_value_t external = data->configuration->genericData;

    switch(data->handler_type) {
        case GET: {
            if (!data->configuration->genericGetter) {
                return jerry_get_property(args_p[0], args_p[1]);
            }

            JerryValueNoRelease name(args_p[1]);
            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

            DEFINE_RETURN_VALUE();
            data->configuration->genericGetter(name.AsLocal<v8::Name>(), info);
            RESTORE_RETURN_VALUE();

            if (!isolate->HasError()) {
                if (*returnValue != isolate->Hole()) {
                    return jerry_acquire_value((*returnValue)->value());
                }

                return jerry_get_property(args_p[0], args_p[1]);
            }
            break;
        }
        case SET: {
            if (!data->configuration->genericSetter) {
                return jerry_set_property(args_p[0], args_p[1], args_p[2]);
            }

            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

            JerryValueNoRelease name(args_p[1]);
            JerryValueNoRelease new_value(args_p[2]);
            v8::Local<v8::Value> v8_value = new_value.AsLocal<v8::Value>();

            DEFINE_RETURN_VALUE();
            data->configuration->genericSetter(name.AsLocal<v8::Name>(), v8_value, info);
            RESTORE_RETURN_VALUE();

            if (!isolate->HasError()) {
                if (*returnValue != isolate->Hole()) {
                    return jerry_create_boolean(true);
                }

                return jerry_set_property(args_p[0], args_p[1], args_p[2]);
            }
            break;
        }
        case QUERY: {
            if (!data->configuration->genericQuery && !data->configuration->genericGetter) {
                return jerry_has_property(args_p[0], args_p[1]);
            }

            JerryValueNoRelease name(args_p[1]);

            if (data->configuration->genericQuery) {
                JerryPropertyCallbackInfo<v8::Integer> info(function_obj, this_val, args_p, args_cnt, external);

                DEFINE_RETURN_VALUE();
                data->configuration->genericQuery(name.AsLocal<v8::Name>(), info);
                RESTORE_RETURN_VALUE();

                if (!isolate->HasError()) {
                    if (*returnValue != isolate->Hole()) {
                        return jerry_create_boolean(true);
                    }

                    return jerry_has_property(args_p[0], args_p[1]);
                }
                break;
            }

            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

            DEFINE_RETURN_VALUE();
            data->configuration->genericGetter(name.AsLocal<v8::Name>(), info);
            RESTORE_RETURN_VALUE();

            if (!isolate->HasError()) {
                if (*returnValue != isolate->Hole()) {
                    return jerry_create_boolean(true);
                }

                return jerry_has_property(args_p[0], args_p[1]);
            }
            break;
        }
        case DELETE: {
            JerryPropertyCallbackInfo<v8::Boolean> info(function_obj, this_val, args_p, args_cnt, external);
            JerryValueNoRelease name(args_p[1]);

            DEFINE_RETURN_VALUE();
            if (data->configuration->genericDeleter) {
                data->configuration->genericDeleter(name.AsLocal<v8::Name>(), info);
            }
            RESTORE_RETURN_VALUE();

            if (!isolate->HasError()) {
                if (*returnValue != isolate->Hole()) {
                    return jerry_acquire_value((*returnValue)->value());
                }

                return jerry_delete_property(args_p[0], args_p[1]);
            }
            break;
        }
        case ENUMERATE: {
            JerryPropertyCallbackInfo<v8::Array> info(function_obj, this_val, args_p, args_cnt, external);

            DEFINE_RETURN_VALUE();
            data->configuration->genericEnumerator(info);
            RESTORE_RETURN_VALUE();

            if (!isolate->HasError()) {
                return jerry_acquire_value((*returnValue)->value());
            }
            break;
        }
        case DEFINE_PROPERTY:
        {
            jerry_property_descriptor_t descriptor;

            jerry_value_t result = jerry_to_property_descriptor(args_p[2], &descriptor);
            if (jerry_value_is_error (result)) {
                return result;
            }
            jerry_release_value(result);

            JerryValueNoRelease name(args_p[1]);
            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

            DEFINE_RETURN_VALUE();

            if (data->configuration->genericDefiner) {
                if (descriptor.is_value_defined) {
                    JerryValue value(jerry_acquire_value(descriptor.value));
                    v8::PropertyDescriptor propertyDescriptor(value.AsLocal<v8::Value>(), descriptor.is_writable);

                    if (descriptor.is_configurable_defined) {
                        propertyDescriptor.set_configurable(descriptor.is_configurable);
                    }

                    if (descriptor.is_enumerable_defined) {
                        propertyDescriptor.set_enumerable(descriptor.is_enumerable);
                    }

                    data->configuration->genericDefiner(name.AsLocal<v8::Name>(), propertyDescriptor, info);
                } else {
                    JerryValue getter(jerry_acquire_value(descriptor.getter));
                    JerryValue setter(jerry_acquire_value(descriptor.setter));
                    v8::PropertyDescriptor propertyDescriptor(getter.AsLocal<v8::Value>(), setter.AsLocal<v8::Value>());

                    if (descriptor.is_configurable_defined) {
                        propertyDescriptor.set_configurable(descriptor.is_configurable);
                    }

                    if (descriptor.is_enumerable_defined) {
                        propertyDescriptor.set_enumerable(descriptor.is_enumerable);
                    }

                    data->configuration->genericDefiner(name.AsLocal<v8::Name>(), propertyDescriptor, info);
                }
            }

            RESTORE_RETURN_VALUE();

            if (!isolate->HasError()) {
                if (*returnValue != isolate->Hole()) {
                    jerry_free_property_descriptor_fields(&descriptor);
                    return jerry_acquire_value((*returnValue)->value());
                }

                jerry_value_t result = jerry_define_own_property(args_p[0], args_p[1], &descriptor);
                jerry_free_property_descriptor_fields(&descriptor);
                return result;
            }

            jerry_free_property_descriptor_fields(&descriptor);
            break;
        }
        case GET_OWN_PROPERTY_DESC:
        {
            if (data->configuration->genericDescriptor) {
                JerryValueNoRelease name(args_p[1]);
                JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

                DEFINE_RETURN_VALUE();
                data->configuration->genericDescriptor(name.AsLocal<v8::Name>(), info);
                RESTORE_RETURN_VALUE();

                if (!isolate->HasError()) {
                    return jerry_acquire_value((*returnValue)->value());
                }
                break;
            }

            if (data->configuration->genericGetter) {
                JerryValueNoRelease name(args_p[1]);
                JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

                DEFINE_RETURN_VALUE();
                data->configuration->genericGetter(name.AsLocal<v8::Name>(), info);
                RESTORE_RETURN_VALUE();

                if (isolate->HasError()) {
                    break;
                }

                if ((*returnValue) != isolate->Hole()) {
                    jerry_value_t result = jerry_create_object();

                    setPropertyDescriptorHelper(result, "value", (*returnValue)->value());
                    setPropertyDescriptorHelperBoolean(result, "writable", true);
                    setPropertyDescriptorHelperBoolean(result, "enumerable", true);
                    setPropertyDescriptorHelperBoolean(result, "configurable", true);
                    return result;
                }
            }

            jerry_value_t result;
            jerry_property_descriptor_t prop_desc;
            jerry_init_property_descriptor_fields(&prop_desc);
            bool status = jerry_get_own_property_descriptor (args_p[0], args_p[1], &prop_desc);

            if (status)
            {
                result = jerry_create_object();

                if (prop_desc.is_value_defined || prop_desc.is_writable_defined)
                {
                    setPropertyDescriptorHelperBoolean(result, "writable", prop_desc.is_writable);
                    setPropertyDescriptorHelper(result, "value", jerry_acquire_value (prop_desc.value));
                }
                else if (prop_desc.is_get_defined || prop_desc.is_set_defined)
                {
                    setPropertyDescriptorHelper(result, "get", jerry_acquire_value (prop_desc.getter));
                    setPropertyDescriptorHelper(result, "set", jerry_acquire_value (prop_desc.setter));
                }

                setPropertyDescriptorHelperBoolean(result, "enumerable", prop_desc.is_enumerable);
                setPropertyDescriptorHelperBoolean(result, "configurable", prop_desc.is_configurable);
            }
            else
            {
                result = jerry_create_undefined ();
            }

            jerry_free_property_descriptor_fields (&prop_desc);
            return result;
        }
    }

    jret = jerry_create_error_from_value(isolate->TakeError(), true);
    return jret;
}
