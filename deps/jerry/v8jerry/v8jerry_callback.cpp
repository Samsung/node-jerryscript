#include "v8jerry_callback.hpp"

#include <v8.h>

#include "v8jerry_value.hpp"
#include "v8jerry_templates.hpp"
#include "v8jerry_isolate.hpp"

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
        : v8::PropertyCallbackInfo<T>(reinterpret_cast<v8::internal::Address*>(BuildArgs(this_val, external_data)))
        , m_args(reinterpret_cast<JerryHandle**>(this->args_))
    {
    }

    ~JerryPropertyCallbackInfo() {
        delete [] m_args;
    }

private:
    static JerryHandle** BuildArgs(const jerry_value_t this_val, const jerry_value_t external_data) {
        JerryHandle **values = new JerryHandle*[kImplicitArgsSize];
        JerryIsolate *isolate = JerryIsolate::fromV8(v8::Isolate::GetCurrent());

        values[v8::PropertyCallbackInfo<T>::kShouldThrowOnErrorIndex] = (JerryHandle*)v8::internal::IntToSmi(v8::internal::Internals::kDontThrow); // TODO: fix this
        values[v8::PropertyCallbackInfo<T>::kHolderIndex] = new JerryValue(jerry_acquire_value(this_val)); // TODO: 'this' object is not correct
        // TODO: correctly fill the arguments:
        values[v8::PropertyCallbackInfo<T>::kIsolateIndex] = reinterpret_cast<JerryHandle*>(isolate);
        //values[v8::PropertyCallbackInfo<T>::kReturnValueDefaultValueIndex] = new JerryValue(jerry_create_undefined()); // TODO
        values[v8::PropertyCallbackInfo<T>::kReturnValueIndex] = isolate->Hole();

        values[v8::PropertyCallbackInfo<T>::kDataIndex] = new JerryValue(jerry_acquire_value(external_data)); /* data; */
        values[v8::PropertyCallbackInfo<T>::kThisIndex] = new JerryValue(jerry_acquire_value(this_val));

        // HandleScope will do the cleanup for us at a later stage
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kHolderIndex]);
        //JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kReturnValueDefaultValueIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kDataIndex]);
        JerryIsolate::GetCurrent()->AddToHandleScope(values[v8::PropertyCallbackInfo<T>::kThisIndex]);

        return values;
    }

    JerryHandle** m_args;
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
        JerryValue new_value(jerry_acquire_value(args_p[0]));

        v8::Local<v8::Value> v8_value = new_value.AsLocal<v8::Value>();

        if (data->is_named) {
            data->v8.setter.named(v8_name, v8_value, info);
        } else {
            data->v8.setter.stringed(v8_name.As<v8::String>(), v8_value, info);
        }
    } else {
        JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, data->external);
        if (data->is_named) {
            data->v8.getter.named(v8_name, info);
        } else {
            data->v8.getter.stringed(v8_name.As<v8::String>(), info);
        }

        JerryIsolate* iso = JerryIsolate::GetCurrent();
        if (!iso->HasError()) {
            v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

            // Again: dragons!
            JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

            jret = jerry_acquire_value(retVal->value());
        } else {
            jret = jerry_create_error_from_value(iso->TakeError(), true);
        }
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
        const jerry_value_t new_target_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt,
        const JerryV8FunctionHandlerData* handlerData)
        // Please note there is a "Hack"/"Fix" in the v8.h file where the FunctionCallbackInfo's "values_" member is accessed!
        : v8::FunctionCallbackInfo<T>(reinterpret_cast<v8::internal::Address*>(implicit_args_data_),
                                      reinterpret_cast<v8::internal::Address*>(BuildArgs(this_val, args_p, args_cnt) + args_cnt - 1),
                                      args_cnt)
    {
        BuildImplicitArgs(function_obj, this_val, new_target_val, handlerData);
    }

    ~JerryFunctionCallbackInfo() {
        /* The values_ points to the end */
        JerryHandle **values = reinterpret_cast<JerryHandle**>(this->values_) - this->Length() + 1;

        delete [] values;
    }

private:
    static JerryValue* newValue(jerry_value_t value) {
        /* Handle scope will release these values */
        JerryValue* jvalue = new JerryValue(jerry_acquire_value(value));
        JerryIsolate::GetCurrent()->AddToHandleScope(jvalue);
        return jvalue;
    }

    static JerryHandle** BuildArgs(
        const jerry_value_t this_val,
        const jerry_value_t args_p[],
        const jerry_length_t args_cnt) {

        JerryHandle **values = new JerryHandle*[args_cnt + 1 /* this */];
        /* this should be at 'values + 1' */

        /* args_p[0]  is at 'values - 0' */
        /* args_p[1]  is at 'values - 1' */
        /* args_p[2]  is at 'values - 2' */
        for (jerry_length_t idx = 0; idx < args_cnt; idx++) {
            values[args_cnt - idx - 1] = newValue(args_p[idx]);
        }
        values[args_cnt] = newValue(this_val);

        return values;
    }

    void BuildImplicitArgs(const jerry_value_t function_obj,
                           const jerry_value_t this_val,
                           const jerry_value_t new_target_val,
                           const JerryV8FunctionHandlerData* handlerData) {
        /* TODO: From the docs:
         * If the callback was created without a Signature, this is the same
         * value as This(). If there is a signature, and the signature didn't match
         * This() but one of its hidden prototypes, this will be the respective
         * hidden prototype.
         */
        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        JerryIsolate* jerry_isolate = reinterpret_cast<JerryIsolate*>(isolate);

        implicit_args_data_[v8::FunctionCallbackInfo<T>::kHolderIndex] = newValue(this_val);
        // TODO: correctly fill the arguments:
        implicit_args_data_[v8::FunctionCallbackInfo<T>::kIsolateIndex] = reinterpret_cast<JerryHandle*>(isolate);
        implicit_args_data_[v8::FunctionCallbackInfo<T>::kReturnValueIndex] = jerry_isolate->Undefined();

        implicit_args_data_[v8::FunctionCallbackInfo<T>::kDataIndex] = newValue(handlerData->function_template->external()); /* data; */
        implicit_args_data_[v8::FunctionCallbackInfo<T>::kNewTargetIndex] = newValue(new_target_val); /* new_target; */
    }

    JerryHandle* implicit_args_data_[kImplicitArgsSize];
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
    // TODO: this should be really checked correcty
    if (!jerry_value_is_undefined(new_target)) {

        // TODO: remove this "constructor" call check.
        if (!jerry_get_object_native_pointer(this_val, NULL, &JerryV8FunctionHandlerData::TypeInfo)) {
            // Add reference to the function template
            // TODO: really do a correct is constructor call check and only set the constructor function template on the new instance.
            //jerry_set_object_native_pointer(this_val, data, &JerryV8FunctionHandlerData::TypeInfo);

            if (data->function_template->HasInstanceTemplate()) {
                JerryObjectTemplate* tmplt = data->function_template->InstanceTemplate();
                tmplt->InstallProperties(this_val);
            }

            // The method was called with a "this" object and there was no "JerryV8ObjectConstructed" set
            // assume that this is a constructo call.
            // TODO: this is not always correct logic. Propert constructor call check is needed.
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
        JerryFunctionCallbackInfo<v8::Value> info(function_obj, this_val, new_target, args_p, args_cnt, data);

        data->v8callback(info);

        JerryIsolate* iso = JerryIsolate::GetCurrent();
        if (!iso->HasError()) {
            v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

            // Again: dragons!
            JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

            jret = jerry_acquire_value(retVal->value());
        } else {
            jret = jerry_create_error_from_value(iso->TakeError(), true);
        }
    }

    // No need to delete the JerryValue here, the HandleScope will take (should) care of it!
    //delete retVal;

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
    JerryIsolate* iso = JerryIsolate::fromV8(v8isolate);

    jerry_value_t jret = jerry_create_undefined();

#define MAKE_NAME(VALUE) JerryValue __name(jerry_acquire_value(VALUE));

    jerry_value_t external = data->configuration->genericData;

    switch(data->handler_type) {
        case GET: {
            if (!data->configuration->genericGetter) {
                return jerry_get_property(args_p[0], args_p[1]);
            }

            MAKE_NAME(args_p[1]);
            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);
            data->configuration->genericGetter(__name.AsLocal<v8::Name>(), info);

            if (!iso->HasError()) {
                v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

                JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

                if (retVal != iso->Hole()) {
                    return jerry_acquire_value(retVal->value());
                }

                return jerry_get_property(args_p[0], args_p[1]);
            }
            break;
        }
        case SET: {
            if (!data->configuration->genericSetter) {
                return jerry_set_property(args_p[0], args_p[1], args_p[2]);
            }

            MAKE_NAME(args_p[1]);
            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

            JerryValue new_value(jerry_acquire_value(args_p[2]));
            v8::Local<v8::Value> v8_value = new_value.AsLocal<v8::Value>();
            data->configuration->genericSetter(__name.AsLocal<v8::Name>(), v8_value, info);

            if (!iso->HasError()) {
                v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

                JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

                if (retVal != iso->Hole()) {
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

            MAKE_NAME(args_p[1]);

            if (data->configuration->genericQuery) {
                JerryPropertyCallbackInfo<v8::Integer> info(function_obj, this_val, args_p, args_cnt, external);
                data->configuration->genericQuery(__name.AsLocal<v8::Name>(), info);

                v8::ReturnValue<v8::Integer> returnValue = info.GetReturnValue();
                JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

                if (!iso->HasError()) {
                    if (retVal != iso->Hole()) {
                        return jerry_create_boolean(true);
                    }

                    return jerry_has_property(args_p[0], args_p[1]);
                }
                break;
            }

            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);
            data->configuration->genericGetter(__name.AsLocal<v8::Name>(), info);

            v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();
            JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

            if (!iso->HasError()) {
                if (retVal != iso->Hole()) {
                    return jerry_create_boolean(true);
                }

                return jerry_has_property(args_p[0], args_p[1]);
            }
            break;
        }
        case DELETE: {
            MAKE_NAME(args_p[1]);
            JerryPropertyCallbackInfo<v8::Boolean> info(function_obj, this_val, args_p, args_cnt, external);

            if (data->configuration->genericDeleter) {
                data->configuration->genericDeleter(__name.AsLocal<v8::Name>(), info);
            }

            if (!iso->HasError()) {
                v8::ReturnValue<v8::Boolean> returnValue = info.GetReturnValue();

                // Again: dragons!
                JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

                if (retVal != iso->Hole()) {
                    return jerry_acquire_value(retVal->value());
                }

                return jerry_delete_property(args_p[0], args_p[1]);
            }
            break;
        }
        case ENUMERATE: {
            JerryPropertyCallbackInfo<v8::Array> info(function_obj, this_val, args_p, args_cnt, external);
            data->configuration->genericEnumerator(info);

            if (!iso->HasError()) {
                v8::ReturnValue<v8::Array> returnValue = info.GetReturnValue();

                // Again: dragons!
                JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);
                return jerry_acquire_value(retVal->value());
            }
            break;
        }
        case DEFINE_PROPERTY:
        {
            MAKE_NAME(args_p[1]);
            JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

            jerry_property_descriptor_t descriptor;

            jerry_value_t result = jerry_to_property_descriptor(args_p[2], &descriptor);
            if (jerry_value_is_error (result)) {
                return result;
            }
            jerry_release_value(result);

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

                    data->configuration->genericDefiner(__name.AsLocal<v8::Name>(), propertyDescriptor, info);
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

                    data->configuration->genericDefiner(__name.AsLocal<v8::Name>(), propertyDescriptor, info);
                }
            }

            if (!iso->HasError()) {
                v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

                // Again: dragons!
                JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

                if (retVal != iso->Hole()) {
                    jerry_free_property_descriptor_fields(&descriptor);
                    return jerry_acquire_value(retVal->value());
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
                MAKE_NAME(args_p[1]);
                JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

                data->configuration->genericDescriptor(__name.AsLocal<v8::Name>(), info);

                if (!iso->HasError()) {
                    v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

                    // Again: dragons!
                    JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

                    return jerry_acquire_value(retVal->value());
                }
                break;
            }

            if (data->configuration->genericGetter) {
                MAKE_NAME(args_p[1]);
                JerryPropertyCallbackInfo<v8::Value> info(function_obj, this_val, args_p, args_cnt, external);

                data->configuration->genericGetter(__name.AsLocal<v8::Name>(), info);

                if (iso->HasError()) {
                    break;
                }

                v8::ReturnValue<v8::Value> returnValue = info.GetReturnValue();

                JerryValue* retVal = **reinterpret_cast<JerryValue***>(&returnValue);

                if (retVal != iso->Hole()) {
                    jerry_value_t result = jerry_create_object();

                    setPropertyDescriptorHelper(result, "value", retVal->value());
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

    jret = jerry_create_error_from_value(iso->TakeError(), true);
    return jret;
}
