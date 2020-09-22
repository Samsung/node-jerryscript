#include "v8.h"
#include "v8-profiler.h"
#include "jerryscript.h"

#include <signal.h>
#include <algorithm>
#include <cassert>
#include <codecvt>
#include <cstring>
#include <deque>
#include <locale>
#include <stack>
#include <string>
#include <vector>

/* Jerry <-> V8 binding classes */
#include "v8jerry_allocator.hpp"
#include "v8jerry_backing_store.hpp"
#include "v8jerry_callback.hpp"
#include "v8jerry_flags.hpp"
#include "v8jerry_handlescope.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_platform.hpp"
#include "v8jerry_templates.hpp"
#include "v8jerry_value.hpp"

// /* Remove the comments to enable trace macros */
// #define USE_TRACE

#if defined(USE_TRACE)
#include <iostream>
#include <sstream>

#define V8_CALL_TRACE() do { _LogTrace(__LINE__, __FILE__, __PRETTY_FUNCTION__); } while (0)

static void _LogTrace(int line, std::string file_name, std::string func_name) {
    size_t last_separator_pos = file_name.find_last_of("/");

    std::ostringstream stream;
    stream << "[Trace]: " << file_name.substr(last_separator_pos + 1) << "(" << line << ") : ";
    stream << func_name << std::endl;

    std::cerr << stream.str();
}

#else
#define V8_CALL_TRACE()
#endif

#define UNIMPLEMENTED(line) V8_CALL_TRACE(); abort()

/* V8 API helpers */
#define RETURN_HANDLE(T, ISOLATE, HANDLE) \
do {                                                                    \
    JerryHandle *__handle = HANDLE;                                    \
    return v8::Local<T>::New(ISOLATE, reinterpret_cast<T*>(__handle)); \
} while (0)


namespace i = v8::internal;

namespace v8 {

SnapshotCreator::SnapshotCreator(Isolate* isolate,
                                 const intptr_t* external_references,
                                 StartupData* existing_snapshot) {
  UNIMPLEMENTED(594);
}

SnapshotCreator::~SnapshotCreator() {
  UNIMPLEMENTED(621);
}

Isolate* SnapshotCreator::GetIsolate() {
  UNIMPLEMENTED(630);
  return NULL;
}

void SnapshotCreator::SetDefaultContext(
    Local<Context> context, SerializeInternalFieldsCallback callback) {
  UNIMPLEMENTED(634);
}

size_t SnapshotCreator::AddContext(Local<Context> context,
                                   SerializeInternalFieldsCallback callback) {
  UNIMPLEMENTED(646);
  return 0;
}

size_t SnapshotCreator::AddData(i::Address object) {
  UNIMPLEMENTED(659);
  return 0;
}

StartupData SnapshotCreator::CreateBlob(
    SnapshotCreator::FunctionCodeHandling function_code_handling) {
  UNIMPLEMENTED(726);
  return StartupData();
}

bool StartupData::CanBeRehashed() const {
  UNIMPLEMENTED(910);
  return false;
}

void V8::SetFlagsFromString(const char* str) {
  UNIMPLEMENTED(919);
}

void V8::SetFlagsFromString(const char* str, size_t length) {
  UNIMPLEMENTED(923);
}

void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) {
  V8_CALL_TRACE();

  for (int idx = 0; idx < *argc; idx++) {
      const char* arg = argv[idx];

      if (strncmp("--", arg, 2) != 0) {
          /* Ignore arguments which does not start with '--' */
          continue;
      }
      arg += 2;

      bool negate = false;

      if (strncmp("no-", arg, 3) == 0) {
          negate = true;
          arg += 3;
      }

      Flag* flag = Flag::Get(arg);

      if (flag == NULL) {
          continue;
      }
      /* Flag found, update it's value */

      if (flag->type == Flag::BOOL) {
          flag->u.bool_value = !negate;
      }

      if (remove_flags) {
          argv[idx] = NULL;
      }
  }

  if (remove_flags) {
      int targetIdx = 0;
      for (int idx = 0; idx < *argc; idx++) {
          if (argv[idx] != NULL) {
              argv[targetIdx++] = argv[idx];
          }
      }
      *argc = targetIdx;
  }
}

void ResourceConstraints::ConfigureDefaults(uint64_t physical_memory,
                                            uint64_t virtual_memory_limit) {
  V8_CALL_TRACE();
}

i::Address* V8::GlobalizeReference(i::Isolate* isolate, i::Address* obj) {
  V8_CALL_TRACE();
  JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(obj);

  JerryHandle* result;
  switch (jhandle->type()) {
      case JerryHandle::GlobalValue:
      case JerryHandle::Value: result = reinterpret_cast<JerryValue*>(jhandle)->CopyToGlobal(); break;
      case JerryHandle::ObjectTemplate:
      case JerryHandle::FunctionTemplate: result = jhandle; break;
      default:
          assert(false && "Unkown Handle type detected");
  }


  return reinterpret_cast<i::Address*>(result);
}

void V8::MoveGlobalReference(internal::Address** from, internal::Address** to) {
  V8_CALL_TRACE();
  // JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(*from);

  // JerryHandle* result;
  // switch (jhandle->type()) {
  //     case JerryHandle::GlobalValue:
  //     case JerryHandle::Value: {
  //       result = reinterpret_cast<JerryValue*>(jhandle)->Move(reinterpret_cast<JerryValue*>(to)); break;
  //       break;
  //     }
  //     case JerryHandle::ObjectTemplate:
  //     case JerryHandle::FunctionTemplate: result = jhandle; break;
  //     default:
  //         assert(false && "Unkown Handle type detected");
  // }
}

bool V8::IsWeak(internal::Address* location)
{
  JerryValue* object = reinterpret_cast<JerryValue*> (location);
  return object->IsWeakReferenced();
}

void V8::MakeWeak(i::Address* location, void* parameter,
                  WeakCallbackInfo<void>::Callback weak_callback,
                  WeakCallbackType type) {
  V8_CALL_TRACE();
  JerryValue* object = reinterpret_cast<JerryValue*> (location);
  assert(object->IsWeakReferenced() == false);

  if (type == WeakCallbackType::kInternalFields) {
      void** wrapper = new void*[kInternalFieldsInWeakCallback + 1];
      for (int i = 0; i < kInternalFieldsInWeakCallback; i++) {
          wrapper[i] = object->GetInternalField<void*>(i);
      }
      wrapper[kInternalFieldsInWeakCallback] = parameter;
      parameter = (void*) wrapper;
  }

  object->MakeWeak(weak_callback, type, parameter);
}

void V8::MakeWeak(i::Address** location_addr) {
  V8_CALL_TRACE();
  JerryValue* object = reinterpret_cast<JerryValue*> (*location_addr);
  assert(object->IsWeakReferenced() == false);
  JerryIsolate::GetCurrent()->AddAsWeak(object);
}

void* V8::ClearWeak(i::Address* location) {
  V8_CALL_TRACE();
  JerryValue* object = reinterpret_cast<JerryValue*> (location);

  return object->ClearWeak();
}

void V8::DisposeGlobal(i::Address* location) {
  JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(location);

  if (JerryHandle::IsValueType(jhandle)) {
      JerryValue* object = reinterpret_cast<JerryValue*> (location);

      if (object->IsWeakReferenced()) {
          object->RunWeakCleanup();
          JerryIsolate::GetCurrent()->RemoveAsWeak(object);
          //object->ClearWeak();
      }

      delete object;
  }
}

Value* V8::Eternalize(Isolate* v8_isolate, Value* value) {
  JerryIsolate* jerry_isolate = reinterpret_cast<JerryIsolate*> (v8_isolate);
  JerryHandle* jerry_handle = reinterpret_cast<JerryHandle*> (value);

  // Just JerryValue has Copy method.
  assert(JerryHandle::IsValueType(jerry_handle));

  int index = -1;
  JerryValue* value_copy = static_cast<JerryValue*>(jerry_handle)->Copy();
  jerry_isolate->SetEternal(value_copy, &index);

  return reinterpret_cast<Value*>(value_copy);
}

void V8::FromJustIsNothing() {
  V8_CALL_TRACE();
}

void V8::ToLocalEmpty() {
  V8_CALL_TRACE();
}

HandleScope::HandleScope(Isolate* isolate)
  : isolate_(reinterpret_cast<internal::Isolate*>(isolate)) {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(isolate_)->PushHandleScope(JerryHandleScopeType::Normal, this);
}

HandleScope::~HandleScope() {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(isolate_)->PopHandleScope(this);
}

i::Address* HandleScope::CreateHandle(i::Isolate* isolate, i::Address value) {
    // V8_CALL_TRACE();
    if (V8_UNLIKELY(value == NULL)) {
        return reinterpret_cast<internal::Address*>(value);
    }
    JerryIsolate* iso = JerryIsolate::fromV8(isolate);

    JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(value);
    switch (jhandle->type()) {
        case JerryHandle::FunctionTemplate:
        case JerryHandle::ObjectTemplate:
            iso->AddTemplate(reinterpret_cast<JerryTemplate*>(jhandle));
            break;
        case JerryHandle::GlobalValue: {
            // In this case a "global" object is copied to a Local<T>, we'll do a copy here and push it to the scope
            // A "global" (Persistent/PersistentBase) should never be in a handle scope
            JerryHandle* jcopy = reinterpret_cast<JerryValue*>(jhandle)->Copy();
            iso->AddToHandleScope(jcopy);
            return reinterpret_cast<internal::Address*>(jcopy);
            break;
        }
        case JerryHandle::Value:
            if (!JerryIsolate::fromV8(isolate)->HasEternal(reinterpret_cast<JerryValue*>(jhandle))) {
                iso->AddToHandleScope(jhandle);
            }
            break;
        default:
            abort();
            break;
    }
    return reinterpret_cast<internal::Address*>(jhandle);
}

EscapableHandleScope::EscapableHandleScope(Isolate* v8_isolate)
  : HandleScope(v8_isolate) {
  V8_CALL_TRACE();
}

i::Address* EscapableHandleScope::Escape(i::Address* escape_value) {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(GetIsolate())->EscapeHandle(reinterpret_cast<JerryValue*>(escape_value));
  return escape_value;
}

SealHandleScope::SealHandleScope(Isolate* isolate)
    : isolate_(reinterpret_cast<v8::internal::Isolate*>(isolate)) {
    V8_CALL_TRACE();
    JerryIsolate::fromV8(isolate_)->PushHandleScope(JerryHandleScopeType::Sealed, this);
}

SealHandleScope::~SealHandleScope() {
    V8_CALL_TRACE();
    JerryIsolate::fromV8(isolate_)->PopHandleScope(this);
}
void Context::Enter() {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  assert(ctx->IsContextObject());
  ctx->ContextEnter();
}

void Context::Exit() {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  assert(ctx->IsContextObject());
  ctx->ContextExit();
}

uint32_t Context::GetNumberOfEmbedderDataFields() {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  return ctx->GetInternalFieldData(0)->count;
}

v8::Local<v8::Value> Context::SlowGetEmbedderData(int index) {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  JerryValue* data = ctx->GetInternalField<JerryValue*>(index);
  RETURN_HANDLE(Value, GetIsolate(), data);
}

void Context::SetEmbedderData(int index, v8::Local<Value> value) {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  ctx->SetInternalField(index, reinterpret_cast<JerryValue*>(*value));
}

void* Context::SlowGetAlignedPointerFromEmbedderData(int index) {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  assert(ctx->IsContextObject());
  return ctx->ContextGetEmbedderData(index);
}

void Context::SetAlignedPointerInEmbedderData(int index, void* value) {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  assert(ctx->IsContextObject());
  ctx->ContextSetEmbedderData(index, value);
}

void Template::Set(v8::Local<Name> name, v8::Local<Data> value,
                   v8::PropertyAttribute attribute) {
  V8_CALL_TRACE();
  JerryTemplate* templt = reinterpret_cast<JerryTemplate*>(this);

  // Here we copy the values as the template's internl elements will do a release on them
  JerryValue* key = reinterpret_cast<JerryValue*>(*name)->Copy();
  JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(*value);

  JerryValue* jvalue;
  switch (jhandle->type()) {
    case JerryHandle::Value:
    case JerryHandle::GlobalValue: {
      jvalue = reinterpret_cast<JerryValue*>(jhandle)->Copy();
      break;
    }
    case JerryHandle::FunctionTemplate: {
      JerryFunctionTemplate* jtmplt = reinterpret_cast<JerryFunctionTemplate*>(jhandle);
      jvalue = jtmplt->GetFunction()->Copy();
      break;
    }
    default: {
      UNIMPLEMENTED(1365);
    }
  }

  templt->Set(key, jvalue, attribute);
}

void Template::SetAccessorProperty(v8::Local<v8::Name> name,
                                   v8::Local<FunctionTemplate> getter,
                                   v8::Local<FunctionTemplate> setter,
                                   v8::PropertyAttribute attribute,
                                   v8::AccessControl access_control) {
  V8_CALL_TRACE();

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

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
  V8_CALL_TRACE();
  JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
  JerryObjectTemplate* obj_template = func->PrototypeTemplate();
  RETURN_HANDLE(ObjectTemplate, Isolate::GetCurrent(), obj_template);
}

void FunctionTemplate::Inherit(v8::Local<FunctionTemplate> value) {
  V8_CALL_TRACE();
  JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
  JerryFunctionTemplate* parent = reinterpret_cast<JerryFunctionTemplate*>(*value);
  func->Inherit(parent->PrototypeTemplate());
}

void FunctionTemplate::SetCallHandler(FunctionCallback callback,
                                      v8::Local<Value> data,
                                      SideEffectType side_effect_type,
                                      const CFunction* not_available_in_node_v14_yet) {
  V8_CALL_TRACE();
  JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
  func->SetCallback(callback);
  if (!data.IsEmpty()) {
      JerryValue* value = reinterpret_cast<JerryValue*>(*data);
      jerry_value_t jvalue = jerry_acquire_value(value->value());

      func->SetExternalData(jvalue);
  }
}

Local<FunctionTemplate> FunctionTemplate::New(
    Isolate* isolate, FunctionCallback callback, v8::Local<Value> data,
    v8::Local<Signature> signature, int length, ConstructorBehavior behavior,
    SideEffectType side_effect_type, const CFunction*) {
  V8_CALL_TRACE();
  // TODO: handle the other args
  JerryFunctionTemplate* func = new JerryFunctionTemplate();
  reinterpret_cast<FunctionTemplate*>(func)->SetCallHandler(callback, data);
  if (!signature.IsEmpty()) {
      func->SetSignature(reinterpret_cast<JerryHandle*>(*signature));
  }

  RETURN_HANDLE(FunctionTemplate, isolate, func);
}

Local<Signature> Signature::New(Isolate* isolate,
                                Local<FunctionTemplate> receiver) {
  V8_CALL_TRACE();
  RETURN_HANDLE(Signature, Isolate::GetCurrent(), reinterpret_cast<JerryFunctionTemplate*>(*receiver));
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  V8_CALL_TRACE();
  JerryFunctionTemplate* func = reinterpret_cast<JerryFunctionTemplate*>(this);
  JerryObjectTemplate* obj_template = func->InstanceTemplate();
  RETURN_HANDLE(ObjectTemplate, Isolate::GetCurrent(), obj_template);
}

void FunctionTemplate::SetClassName(Local<String> name) {
  V8_CALL_TRACE();
  JerryFunctionTemplate* tmplt = reinterpret_cast<JerryFunctionTemplate*>(this);
  // Trigger creation of the prototype object if it was not done yet:
  (void)tmplt->PrototypeTemplate();
  // TODO: This should be used as the constructor's name. Skip this for now.
}

Local<ObjectTemplate> ObjectTemplate::New(
    Isolate* isolate, v8::Local<FunctionTemplate> constructor) {
  V8_CALL_TRACE();
  JerryObjectTemplate* obj_template = new JerryObjectTemplate();

  if (!constructor.IsEmpty()) {
      // TODO: check if this is correct
      JerryFunctionTemplate* function_template = reinterpret_cast<JerryFunctionTemplate*>(*constructor);
      obj_template->SetConstructor(function_template);
  }

  RETURN_HANDLE(ObjectTemplate, isolate, obj_template);
}

void ObjectTemplate::SetAccessor(v8::Local<String> name,
                                 AccessorGetterCallback getter,
                                 AccessorSetterCallback setter,
                                 v8::Local<Value> data, AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
    V8_CALL_TRACE();
    JerryObjectTemplate* tmplt = reinterpret_cast<JerryObjectTemplate*>(this);

    JerryValue* jname = reinterpret_cast<JerryValue*>(*name)->Copy();
    JerryValue* jdata = NULL;
    if (!data.IsEmpty()) {
        jdata = reinterpret_cast<JerryValue*>(*data)->Copy();
    }

    tmplt->SetAccessor(jname, getter, setter, jdata, settings, attribute);
}

void ObjectTemplate::SetAccessor(v8::Local<Name> name,
                                 AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 v8::Local<Value> data, AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
  V8_CALL_TRACE();
  JerryObjectTemplate* tmplt = reinterpret_cast<JerryObjectTemplate*>(this);

  JerryValue* jname = reinterpret_cast<JerryValue*>(*name)->Copy();
  JerryValue* jdata = NULL;
  if (!data.IsEmpty()) {
      jdata = reinterpret_cast<JerryValue*>(*data)->Copy();
  }

  tmplt->SetAccessor(jname, getter, setter, jdata, settings, attribute);
}

void ObjectTemplate::SetHandler(
    const NamedPropertyHandlerConfiguration& config) {
  V8_CALL_TRACE();
  JerryObjectTemplate* object_template = reinterpret_cast<JerryObjectTemplate*>(this);
  object_template->SetProxyHandler(config);

}

void ObjectTemplate::SetHandler(
    const IndexedPropertyHandlerConfiguration& config) {
  V8_CALL_TRACE();
  // [[TODO]] interceptor!!
}

void ObjectTemplate::SetInternalFieldCount(int value) {
  V8_CALL_TRACE();
  JerryObjectTemplate* tmplt = reinterpret_cast<JerryObjectTemplate*>(this);
  tmplt->SetInteralFieldCount(value);
}

ScriptCompiler::CachedData::CachedData(const uint8_t* data_, int length_,
                                       BufferPolicy buffer_policy_)
    : data(data_),
      length(length_),
      rejected(false),
      buffer_policy(buffer_policy_) {
  UNIMPLEMENTED(2037);
}

ScriptCompiler::CachedData::~CachedData() {
  V8_CALL_TRACE();
}

struct UnboundScriptData {
  UnboundScriptData(Isolate* isolate) : isolate(isolate) {}

  Isolate* isolate;
};

void unboundScriptFreeCallback(void *native_p) {
  delete (UnboundScriptData*)native_p;
}

static jerry_object_native_info_t unboundScriptInfo {
  .free_cb = unboundScriptFreeCallback
};

Local<Script> UnboundScript::BindToCurrentContext() {
  V8_CALL_TRACE();
  const JerryValue* unboundScript = reinterpret_cast<const JerryValue*>(this);
  void* unboundScriptData;
  jerry_get_object_native_pointer(unboundScript->value(), &unboundScriptData, &unboundScriptInfo);
  RETURN_HANDLE(Script, ((UnboundScriptData*)unboundScriptData)->isolate, new JerryValue(jerry_acquire_value(unboundScript->value())));
}

MaybeLocal<Value> Script::Run(Local<Context> context) {
  V8_CALL_TRACE();
  const JerryValue* script = reinterpret_cast<const JerryValue*>(this);

  jerry_value_t result = jerry_run(script->value());

  RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(result));
}

Local<PrimitiveArray> ScriptOrModule::GetHostDefinedOptions() {
  UNIMPLEMENTED(2164);
  return Local<PrimitiveArray>();
}

Local<PrimitiveArray> PrimitiveArray::New(Isolate* v8_isolate, int length) {
  if (length < 0) {
      length = 0;
  }

  jerry_value_t array_value = jerry_create_array(length);
  RETURN_HANDLE(PrimitiveArray, v8_isolate, new JerryValue(array_value));
}

int PrimitiveArray::Length() const {
  V8_CALL_TRACE();
  const JerryValue* array = reinterpret_cast<const JerryValue*>(this);

  return jerry_get_array_length(array->value());
}

void PrimitiveArray::Set(Isolate* v8_isolate, int index,
                         Local<Primitive> item) {
  V8_CALL_TRACE();
  const JerryValue* array = reinterpret_cast<const JerryValue*>(this);
  const JerryValue* value = reinterpret_cast<const JerryValue*>(*item);

  jerry_set_property_by_index(array->value(), (uint32_t)index, value->value());
}

Local<Primitive> PrimitiveArray::Get(Isolate* v8_isolate, int index) {
  V8_CALL_TRACE();
  const JerryValue* array = reinterpret_cast<const JerryValue*>(this);

  jerry_value_t property = jerry_get_property_by_index(array->value(), (uint32_t)index);

  JerryValue* result = NULL;
  if (!jerry_value_is_error(property)) {
      result = new JerryValue(property);
  } else {
      jerry_release_value(property);
  }

  RETURN_HANDLE(Primitive, v8_isolate, result);
}

Module::Status Module::GetStatus() const {
  UNIMPLEMENTED(2219);
  return kUninstantiated;
}

Local<Value> Module::GetException() const {
  UNIMPLEMENTED(2239);
  return Local<Value>();
}

int Module::GetModuleRequestsLength() const {
  UNIMPLEMENTED(2247);
  return 0;
}

Local<String> Module::GetModuleRequest(int i) const {
  UNIMPLEMENTED(2256);
  return Local<String>();
}

Local<Value> Module::GetModuleNamespace() {
  UNIMPLEMENTED(2288);
  return Local<Value>();
}

Local<UnboundModuleScript> Module::GetUnboundModuleScript() {
  UNIMPLEMENTED(2298);
  return Local<UnboundModuleScript>();
}

int Module::GetIdentityHash() const {
  V8_CALL_TRACE();
  const JerryValue* module = reinterpret_cast<const JerryValue*>(this);
  return (int)module->value();
}

Maybe<bool> Module::InstantiateModule(Local<Context> context,
                                      Module::ResolveCallback callback) {
  V8_CALL_TRACE();
  return Just(true);
}

MaybeLocal<Value> Module::Evaluate(Local<Context> context) {
  V8_CALL_TRACE();
  RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(jerry_create_undefined()));
}

Local<Module> Module::CreateSyntheticModule(
    Isolate* isolate, Local<String> module_name,
    const std::vector<Local<v8::String>>& export_names,
    v8::Module::SyntheticModuleEvaluationSteps evaluation_steps) {
  V8_CALL_TRACE();
  String::Utf8Value fileName(isolate, module_name);

  jerry_value_t scriptFunction = jerry_parse((const jerry_char_t*)*fileName,
                                              module_name->Utf8Length(isolate),
                                              (const jerry_char_t*)"",
                                              0,
                                              JERRY_PARSE_NO_OPTS | 2); // [[TODO]] propagete ECMA_PARSE_MODULE to api

  JerryValue* result = JerryValue::TryCreateValue(JerryIsolate::fromV8(isolate), scriptFunction);
  RETURN_HANDLE(Module, isolate, result);
}

Maybe<bool> Module::SetSyntheticModuleExport(Isolate* isolate,
                                             Local<String> export_name,
                                             Local<v8::Value> export_value) {
  V8_CALL_TRACE();
  return Just(true);
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundScript(
    Isolate* v8_isolate, Source* source, CompileOptions options,
    NoCacheReason no_cache_reason) {
  V8_CALL_TRACE();

  if (options == CompileOptions::kConsumeCodeCache) {
    String::Utf8Value text(v8_isolate, source->source_string);
    CachedData* data = source->cached_data;
    if (data->length != text.length() || memcmp(data->data, (const uint8_t*) *text, text.length())) {
        data->rejected = true;
    }
  }

  Local<String> file;

  if (source->resource_name.IsEmpty()) {
    file = source->resource_name.As<String>();
  } else {
    bool isOk =source->resource_name->ToString(v8_isolate->GetCurrentContext()).ToLocal(&file);

    if (!isOk) {
      return MaybeLocal<UnboundScript>();
    }
  }

  String::Utf8Value text(v8_isolate, source->source_string);
  String::Utf8Value fileName(v8_isolate,file);

  jerry_value_t scriptFunction = jerry_parse((const jerry_char_t*)*fileName,
                                              file->Utf8Length(v8_isolate),
                                              (const jerry_char_t*)*text,
                                              source->source_string->Utf8Length(v8_isolate),
                                              JERRY_PARSE_NO_OPTS);

  JerryValue* result = JerryValue::TryCreateValue(JerryIsolate::fromV8(v8_isolate), scriptFunction);

  UnboundScriptData *unboundScriptData = new UnboundScriptData(v8_isolate);
  jerry_set_object_native_pointer(scriptFunction, unboundScriptData, &unboundScriptInfo);

  RETURN_HANDLE(UnboundScript, v8_isolate, result);
}

MaybeLocal<Module> ScriptCompiler::CompileModule(
    Isolate* isolate, Source* source, CompileOptions options,
    NoCacheReason no_cache_reason) {
  V8_CALL_TRACE();

  if (options == CompileOptions::kConsumeCodeCache) {
    String::Utf8Value text(isolate, source->source_string);
    CachedData* data = source->cached_data;
    if (data->length != text.length() || memcmp(data->data, (const uint8_t*) *text, text.length())) {
        data->rejected = true;
    }
  }

  Local<String> file;

  if (source->resource_name.IsEmpty()) {
    file = source->resource_name.As<String>();
  } else {
    bool isOk =source->resource_name->ToString(isolate->GetCurrentContext()).ToLocal(&file);

    if (!isOk) {
      return MaybeLocal<Module>();
    }
  }

  String::Utf8Value text(isolate, source->source_string);
  String::Utf8Value fileName(isolate, file);

  jerry_value_t scriptFunction = jerry_parse((const jerry_char_t*)*fileName,
                                              file->Utf8Length(isolate),
                                              (const jerry_char_t*)*text,
                                              source->source_string->Utf8Length(isolate),
                                              JERRY_PARSE_NO_OPTS | 2); // [[TODO]] propagete ECMA_PARSE_MODULE to api

  JerryValue* result = JerryValue::TryCreateValue(JerryIsolate::fromV8(isolate), scriptFunction);
  RETURN_HANDLE(Module, isolate, result);
}

MaybeLocal<Function> ScriptCompiler::CompileFunctionInContext(
    Local<Context> v8_context, Source* source, size_t arguments_count,
    Local<String> arguments[], size_t context_extension_count,
    Local<Object> context_extensions[], CompileOptions options,
    NoCacheReason no_cache_reason,
    Local<ScriptOrModule>* script_or_module_out) {
  V8_CALL_TRACE();

  Isolate *isolate = v8_context->GetIsolate();

  if (options == CompileOptions::kConsumeCodeCache) {
    String::Utf8Value text(isolate, source->source_string);
    CachedData* data = source->cached_data;
    if (data->length != text.length() || memcmp(data->data, (const uint8_t*) *text, text.length())) {
        data->rejected = true;
    }
  }

  Local<String> file;
  if (source->resource_name.IsEmpty()) {
    file = source->resource_name.As<String>();
  } else {
    bool isOk =source->resource_name->ToString(isolate->GetCurrentContext()).ToLocal(&file);

    if (!isOk) {
      return MaybeLocal<Function>();
    }
  }

  std::string args;

  for (size_t i = 0; i < arguments_count; i++) {
    String::Utf8Value arg(isolate, arguments[i]);
    args.append((const char*)*arg, (size_t) arguments[i]->Utf8Length(isolate));
    if (i != arguments_count - 1) {
      args.append(",", 1);
    }
  }

  String::Utf8Value text(isolate, source->source_string);
  String::Utf8Value fileName(isolate, file);

  jerry_value_t scriptFunction = jerry_parse_function((const jerry_char_t*)*fileName,
                                                      file->Utf8Length(isolate),
                                                      (const jerry_char_t*) args.c_str(),
                                                      args.length(),
                                                      (const jerry_char_t*)*text,
                                                      source->source_string->Utf8Length(isolate),
                                                      JERRY_PARSE_NO_OPTS);

  if (script_or_module_out != nullptr) {
    jerry_value_t object = jerry_create_object();
    JerryValue* script_or_module = JerryValue::TryCreateValue(JerryIsolate::fromV8(isolate), object);
    *script_or_module_out = Local<ScriptOrModule>::New(isolate, reinterpret_cast<ScriptOrModule*>(script_or_module));
  }

  JerryValue* result = JerryValue::TryCreateValue(JerryIsolate::fromV8(isolate), scriptFunction);
  RETURN_HANDLE(Function, isolate, result);
}

uint32_t ScriptCompiler::CachedDataVersionTag() {
  V8_CALL_TRACE();
  return 0;
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundScript> unbound_script) {
  V8_CALL_TRACE();
  return new CachedData();
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundModuleScript> unbound_module_script) {
  V8_CALL_TRACE();
  return new CachedData();
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCacheForFunction(
    Local<Function> function) {
  V8_CALL_TRACE();
  return new CachedData();
}

MaybeLocal<Script> Script::Compile(Local<Context> context, Local<String> source,
                                   ScriptOrigin* origin) {
  UNIMPLEMENTED(2700);
  return MaybeLocal<Script>();
}

v8::TryCatch::TryCatch(v8::Isolate* isolate)
    : isolate_(reinterpret_cast<v8::internal::Isolate*>(isolate)),
      next_(NULL),
      is_verbose_(false),
      can_continue_(true),
      capture_message_(true),
      rethrow_(false),
      has_terminated_(false) {
  V8_CALL_TRACE();

  JerryIsolate::fromV8(isolate_)->PushTryCatch(this);
}

v8::TryCatch::~TryCatch() {
    V8_CALL_TRACE();
    // TODO: remove from isolate stack
    JerryIsolate* iso = JerryIsolate::fromV8(isolate_);
    iso->PopTryCatch(this);

    if (!rethrow_ && HasCaught()) {
        if (is_verbose_) {
            JerryValue error(jerry_acquire_value(iso->GetRawError()->value()));
            iso->ClearError();
            Local<Value> exception(reinterpret_cast<Value*>(&error));

            // Replace "stack" property on exception as V8 creates a stack string and not an array.
            iso->UpdateErrorStackProp(error);

            Local<v8::Message> message;
            iso->ReportMessage(message, exception);
        } else {
            // By not clearing the error "effectively" the parent TryCatch would inherit everything.
            iso->ClearError();
        }
    }
}

bool v8::TryCatch::HasCaught() const {
  V8_CALL_TRACE();
  // TODO: return error from isolate/context
  return JerryIsolate::fromV8(isolate_)->HasError();
}

bool v8::TryCatch::CanContinue() const {
  V8_CALL_TRACE();
  return can_continue_;
}

bool v8::TryCatch::HasTerminated() const {
  V8_CALL_TRACE();;
  return has_terminated_;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  V8_CALL_TRACE();
  rethrow_ = true;

  // TODO: push error to "try catch" stack above?
  // TODO: return what?
  return Local<Value>();
}

v8::Local<Value> v8::TryCatch::Exception() const {
  V8_CALL_TRACE();
  // TODO: return the current error object
  return Local<Value>(reinterpret_cast<Value*>(JerryIsolate::fromV8(isolate_)->GetRawError()));
}

v8::Local<v8::Message> v8::TryCatch::Message() const {
  V8_CALL_TRACE();
  // TODO: return the current error message
  return Local<v8::Message>();
}

void v8::TryCatch::SetVerbose(bool value) {
  V8_CALL_TRACE();
  is_verbose_ = value;
}

bool v8::TryCatch::IsVerbose() const {
  V8_CALL_TRACE();
    return is_verbose_;
}

Local<String> Message::Get() const {
  UNIMPLEMENTED(2843);
  return Local<String>();
}

v8::Isolate* Message::GetIsolate() const {
  return JerryIsolate::toV8(JerryIsolate::GetCurrent());
}

ScriptOrigin Message::GetScriptOrigin() const {
  V8_CALL_TRACE();

  JerryValue* arr[] = {
    new JerryValue(jerry_create_undefined()),
    new JerryValue(jerry_create_number(0)),
    new JerryValue(jerry_create_number(0)),
    new JerryValue(jerry_create_boolean(false)),
    new JerryValue(jerry_create_number(0)),
    new JerryValue(jerry_create_undefined()),
    new JerryValue(jerry_create_boolean(false)),
    new JerryValue(jerry_create_boolean(false)),
    new JerryValue(jerry_create_boolean(false)),
    new JerryValue(jerry_create_array(0)),
  };

  return ScriptOrigin(arr[0]->AsLocal<Value>(),
                      arr[1]->AsLocal<Integer>(),
                      arr[2]->AsLocal<Integer>(),
                      arr[3]->AsLocal<Boolean>(),
                      arr[4]->AsLocal<Integer>(),
                      arr[5]->AsLocal<Value>(),
                      arr[6]->AsLocal<Boolean>(),
                      arr[7]->AsLocal<Boolean>(),
                      arr[8]->AsLocal<Boolean>(),
                      arr[9]->AsLocal<PrimitiveArray>());
}

v8::Local<Value> Message::GetScriptResourceName() const {
  V8_CALL_TRACE();
  return v8::Local<Value>();
}

v8::Local<v8::StackTrace> Message::GetStackTrace() const {
  V8_CALL_TRACE();
  return Local<v8::StackTrace>();
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  V8_CALL_TRACE();
  return Just(0);
}

int Message::ErrorLevel() const {
  V8_CALL_TRACE();
  return v8::Isolate::MessageErrorLevel::kMessageError;
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  V8_CALL_TRACE();
  return Just(0);
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  V8_CALL_TRACE();
  return Just(0);
}

MaybeLocal<String> Message::GetSourceLine(Local<Context> context) const {
  V8_CALL_TRACE();
  return MaybeLocal<String>();
}

Local<StackFrame> StackTrace::GetFrame(Isolate* v8_isolate,
                                       uint32_t index) const {
  V8_CALL_TRACE();
  return Local<StackFrame>();
}

int StackTrace::GetFrameCount() const {
  V8_CALL_TRACE();
  return 0;
}

Local<StackTrace> StackTrace::CurrentStackTrace(Isolate* isolate,
                                                int frame_limit,
                                                StackTraceOptions options) {
  V8_CALL_TRACE();
  return Local<StackTrace>();
}

int StackFrame::GetLineNumber() const {
  V8_CALL_TRACE();
  return 0;
}

int StackFrame::GetColumn() const {
  V8_CALL_TRACE();
  return 0;
}

int StackFrame::GetScriptId() const {
  V8_CALL_TRACE();
  return 0;
}

Local<String> StackFrame::GetScriptName() const {
  V8_CALL_TRACE();
  return Local<String>();
}

Local<String> StackFrame::GetFunctionName() const {
  V8_CALL_TRACE();
  return Local<String>();
}

bool StackFrame::IsEval() const {
  V8_CALL_TRACE();
  return false;
}

MaybeLocal<Value> JSON::Parse(Local<Context> context,
                              Local<String> json_string) {
  UNIMPLEMENTED(3082);
  return MaybeLocal<Value>();
}

MaybeLocal<String> JSON::Stringify(Local<Context> context,
                                   Local<Value> json_object,
                                   Local<String> gap) {
  UNIMPLEMENTED(3097);
  return MaybeLocal<String>();
}

Maybe<bool> ValueSerializer::Delegate::WriteHostObject(Isolate* v8_isolate,
                                                       Local<Object> object) {
  UNIMPLEMENTED(3119);
  return Just(false);
}

Maybe<uint32_t> ValueSerializer::Delegate::GetSharedArrayBufferId(
    Isolate* v8_isolate, Local<SharedArrayBuffer> shared_array_buffer) {
  UNIMPLEMENTED(3128);
  return Just((uint32_t) 0);
}

void Isolate::SetAllowWasmCodeGenerationCallback(
      AllowWasmCodeGenerationCallback callback) {
  V8_CALL_TRACE();
}

Maybe<uint32_t> ValueSerializer::Delegate::GetWasmModuleTransferId(
    Isolate* v8_isolate, Local<WasmModuleObject> module) {
  UNIMPLEMENTED(3137);
  return Just((uint32_t) 0);
}

void* ValueSerializer::Delegate::ReallocateBufferMemory(void* old_buffer,
                                                        size_t size,
                                                        size_t* actual_size) {
  UNIMPLEMENTED(3142);
  return NULL;
}

void ValueSerializer::Delegate::FreeBufferMemory(void* buffer) {
  UNIMPLEMENTED(3149);
}

ValueSerializer::ValueSerializer(Isolate* isolate, Delegate* delegate)
    : private_(NULL) {
  UNIMPLEMENTED(3163);
}

ValueSerializer::~ValueSerializer() {
  UNIMPLEMENTED(3167);
}

void ValueSerializer::WriteHeader() {
  UNIMPLEMENTED(3169);
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {
  UNIMPLEMENTED(3171);
}

Maybe<bool> ValueSerializer::WriteValue(Local<Context> context,
                                        Local<Value> value) {
  UNIMPLEMENTED(3175);
  return Just(false);
}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
  UNIMPLEMENTED(3187);
  return std::pair<uint8_t*, size_t>(NULL, 0);
}

void ValueSerializer::TransferArrayBuffer(uint32_t transfer_id,
                                          Local<ArrayBuffer> array_buffer) {
  UNIMPLEMENTED(3191);
}

void ValueSerializer::WriteUint32(uint32_t value) {
  UNIMPLEMENTED(3197);
}

void ValueSerializer::WriteUint64(uint64_t value) {
  UNIMPLEMENTED(3201);
}

void ValueSerializer::WriteDouble(double value) {
  UNIMPLEMENTED(3205);
}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
  UNIMPLEMENTED(3209);
}

MaybeLocal<Object> ValueDeserializer::Delegate::ReadHostObject(
    Isolate* v8_isolate) {
  UNIMPLEMENTED(3213);
  return MaybeLocal<Object>();
}

MaybeLocal<WasmModuleObject> ValueDeserializer::Delegate::GetWasmModuleFromId(
    Isolate* v8_isolate, uint32_t id) {
  UNIMPLEMENTED(3222);
  return MaybeLocal<WasmModuleObject>();
}

MaybeLocal<SharedArrayBuffer>
ValueDeserializer::Delegate::GetSharedArrayBufferFromId(Isolate* v8_isolate,
                                                        uint32_t id) {
  UNIMPLEMENTED(3231);
  return MaybeLocal<SharedArrayBuffer>();
}

ValueDeserializer::ValueDeserializer(Isolate* isolate, const uint8_t* data,
                                     size_t size, Delegate* delegate) {
  UNIMPLEMENTED(3254);
}

ValueDeserializer::~ValueDeserializer() {
  UNIMPLEMENTED(3267);
}

Maybe<bool> ValueDeserializer::ReadHeader(Local<Context> context) {
  UNIMPLEMENTED(3269);
  return Just(false);
}

uint32_t ValueDeserializer::GetWireFormatVersion() const {
  UNIMPLEMENTED(3305);
  return 0;
}

MaybeLocal<Value> ValueDeserializer::ReadValue(Local<Context> context) {
  UNIMPLEMENTED(3310);
  return MaybeLocal<Value>();
}

void ValueDeserializer::TransferArrayBuffer(uint32_t transfer_id,
                                            Local<ArrayBuffer> array_buffer) {
  UNIMPLEMENTED(3326);
}

void ValueDeserializer::TransferSharedArrayBuffer(
    uint32_t transfer_id, Local<SharedArrayBuffer> shared_array_buffer) {
  UNIMPLEMENTED(3333);
}

bool ValueDeserializer::ReadUint32(uint32_t* value) {
  UNIMPLEMENTED(3340);
  return false;
}

bool ValueDeserializer::ReadUint64(uint64_t* value) {
  UNIMPLEMENTED(3344);
  return false;
}

bool ValueDeserializer::ReadDouble(double* value) {
  UNIMPLEMENTED(3348);
  return false;
}

bool ValueDeserializer::ReadRawBytes(size_t length, const void** data) {
  UNIMPLEMENTED(3352);
  return false;
}

bool Value::FullIsUndefined() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsUndefined();
}

bool Value::FullIsNull() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsNull();
}

bool Value::IsTrue() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsTrue();
}

bool Value::IsFalse() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsFalse();
}

bool Value::IsFunction() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsFunction();
}

bool Value::IsName() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsName();
}

bool Value::FullIsString() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsString();
}

bool Value::IsSymbol() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsSymbol();
}

bool Value::IsArray() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsArray();
}

bool Value::IsArrayBuffer() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsArrayBuffer();
}

bool Value::IsArrayBufferView() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsArrayBufferView();
}

bool Value::IsDataView() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsDataView();
}

bool Value::IsObject() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsObject();
}

bool Value::IsNumber() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsNumber();
}

bool Value::IsBigInt() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsBigInt();
}

bool Value::IsProxy() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsProxy();
}

bool Value::IsBoolean() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsBoolean();
}

bool Value::IsExternal() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsExternal();
}

bool Value::IsInt32() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsInt32();
}

bool Value::IsUint32() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsUint32();
}

bool Value::IsNativeError() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsNativeError();
}

bool Value::IsPromise() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsPromise();
}

bool Value::IsModuleNamespaceObject() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*> (this)->IsModuleNameSpaceObject();
}

MaybeLocal<String> Value::ToString(Local<Context> context) const {
  V8_CALL_TRACE();
  JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToString();
  RETURN_HANDLE(String, context->GetIsolate(), result);
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
  V8_CALL_TRACE();
  return ToString(context);
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
  V8_CALL_TRACE();
  JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToObject();
  RETURN_HANDLE(Object, Isolate::GetCurrent(), result);
}

bool Value::BooleanValue(Isolate* v8_isolate) const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->BooleanValue();
}

Local<Boolean> Value::ToBoolean(Isolate* v8_isolate) const {
  V8_CALL_TRACE();
  JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToBoolean();
  RETURN_HANDLE(Boolean, v8_isolate, result);
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
  V8_CALL_TRACE();
  JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToNumber();
  RETURN_HANDLE(Number, context->GetIsolate(), result);
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
  V8_CALL_TRACE();
  JerryValue* result = reinterpret_cast<const JerryValue*>(this)->ToInteger();
  RETURN_HANDLE(Integer, context->GetIsolate(), result);
}

i::Isolate* i::IsolateFromNeverReadOnlySpaceObject(i::Address obj) {
  UNIMPLEMENTED(3633);
  return NULL;
}

bool i::ShouldThrowOnError(i::Isolate* isolate) {
  UNIMPLEMENTED(3637);
  return false;
}

void i::Internals::CheckInitializedImpl(v8::Isolate* external_isolate) {
  V8_CALL_TRACE();
}

void External::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Object::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::Function::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::Boolean::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Name::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::String::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Symbol::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Private::CheckCast(v8::Data* that) {
  V8_CALL_TRACE();
}

void v8::Number::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Integer::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Int32::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Uint32::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::BigInt::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

void v8::Array::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::Map::CheckCast(Value* that) {
  V8_CALL_TRACE();;
}

void v8::Promise::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::Promise::Resolver::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::Proxy::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

v8::BackingStore::~BackingStore() {
  V8_CALL_TRACE();
}

void* v8::BackingStore::Data() const {
  V8_CALL_TRACE();
  const JerryBackingStore *backingStore = JerryBackingStore::fromV8(this);
  return backingStore->data();
}

size_t v8::BackingStore::ByteLength() const {
  V8_CALL_TRACE();
  const JerryBackingStore *backingStore = JerryBackingStore::fromV8(this);
  return backingStore->byteLength();
}

std::shared_ptr<v8::BackingStore> v8::ArrayBuffer::GetBackingStore() {
  V8_CALL_TRACE();
  JerryBackingStore* jerryBackingStore = reinterpret_cast<JerryValue*>(this)->GetBackingStore();
  return std::shared_ptr<v8::BackingStore>{reinterpret_cast<v8::BackingStore*>(new BackingStoreRef(jerryBackingStore))};
}

std::shared_ptr<v8::BackingStore> v8::SharedArrayBuffer::GetBackingStore() {
  V8_CALL_TRACE();
  JerryBackingStore* jerryBackingStore = reinterpret_cast<JerryValue*>(this)->GetBackingStore();
  return std::shared_ptr<v8::BackingStore>{reinterpret_cast<v8::BackingStore*>(new BackingStoreRef(jerryBackingStore))};
}

void v8::ArrayBuffer::CheckCast(Value* that) {
  V8_CALL_TRACE();;
}

void v8::ArrayBufferView::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::TypedArray::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::DataView::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::SharedArrayBuffer::CheckCast(Value* that) {
  V8_CALL_TRACE();
}

void v8::Date::CheckCast(v8::Value* that) {
  V8_CALL_TRACE();
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
  UNIMPLEMENTED(3905);
  return Just((double) 0);
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
  V8_CALL_TRACE();
  int64_t result = 0;
  JerryValue* convertedValue = reinterpret_cast<const JerryValue*>(this)->ToInteger();

  if (convertedValue != NULL) {
    result = convertedValue->GetInt64Value();
    delete convertedValue;
  }

  return Just(result);
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
  V8_CALL_TRACE();
  int32_t result = 0;
  JerryValue* convertedValue = reinterpret_cast<const JerryValue*>(this)->ToInteger();

  if (convertedValue != NULL) {
    result = convertedValue->GetInt32Value();
    delete convertedValue;
  }

  return Just(result);
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
  V8_CALL_TRACE();
  uint32_t result = 0;
  // TODO: Use ToUint32 conversion if available.
  JerryValue* convertedValue = reinterpret_cast<const JerryValue*>(this)->ToInteger();

  if (convertedValue != NULL) {
    double value = convertedValue->GetNumberValue();

    result = (value >= 0) ? (uint32_t)value : 0;
    delete convertedValue;
  }

  return Just(result);
}

bool Value::StrictEquals(Local<Value> that) const {
  V8_CALL_TRACE();
  const JerryValue* lhs = reinterpret_cast<const JerryValue*> (this);
  JerryValue* rhs = reinterpret_cast<JerryValue*> (*that);

  jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_STRICT_EQUAL, lhs->value(), rhs->value());
  bool isEqual = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
  jerry_release_value(result);

  return isEqual;
}

bool Value::SameValue(Local<Value> that) const {
  UNIMPLEMENTED(3995);
  return false;
}

Maybe<bool> Value::InstanceOf(v8::Local<v8::Context> context,
                              v8::Local<v8::Object> object) {
  UNIMPLEMENTED(4008);
  return Just(false);
}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context,
                            v8::Local<Value> key, v8::Local<Value> value) {
  V8_CALL_TRACE();
  return Just(reinterpret_cast<JerryValue*>(this)->SetProperty(
                  reinterpret_cast<JerryValue*>(*key),
                  reinterpret_cast<JerryValue*>(*value)));
}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context, uint32_t index,
                            v8::Local<Value> value) {
  V8_CALL_TRACE();
  return Just(reinterpret_cast<JerryValue*>(this)->SetPropertyIdx(index, reinterpret_cast<JerryValue*>(*value)));
}

/* PropertyDescriptor */

struct PropertyDescriptor::PrivateData {
    bool enumerable : 1;
    bool has_enumerable : 1;
    bool configurable : 1;
    bool has_configurable : 1;
    bool writable : 1;
    bool has_writable : 1;
    Local<Value> value;
    Local<Value> get;
    Local<Value> set;

    PrivateData()
        : enumerable(false)
        , has_enumerable(false)
        , configurable(false)
        , has_configurable(false)
        , writable(false)
        , has_writable(false)
    {}
};

v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value)
    : private_(new PrivateData()) {
  private_->value = value;
}

v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value,
                                           bool writable)
    : private_(new PrivateData()) {
  private_->value = value;
  private_->has_writable = true;
  private_->writable = writable;
}

v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> get,
                                           v8::Local<v8::Value> set)
    : private_(new PrivateData()) {
  private_->get = get;
  private_->set = set;;
}

v8::PropertyDescriptor::~PropertyDescriptor() {
  delete reinterpret_cast<PrivateData*>(private_);
}

v8::Local<Value> v8::PropertyDescriptor::value() const {
  return private_->value;
}

v8::Local<Value> v8::PropertyDescriptor::get() const {
  UNIMPLEMENTED(4123);
  return Local<Value>();
}

v8::Local<Value> v8::PropertyDescriptor::set() const {
  return private_->get;
}

bool v8::PropertyDescriptor::has_value() const {
  return !private_->value.IsEmpty();
}

bool v8::PropertyDescriptor::has_get() const {
  return !private_->get.IsEmpty();
}

bool v8::PropertyDescriptor::has_set() const {
  return !private_->set.IsEmpty();
}

bool v8::PropertyDescriptor::writable() const {
  return private_->writable;
}

bool v8::PropertyDescriptor::has_writable() const {
  return private_->has_writable;
}

void v8::PropertyDescriptor::set_enumerable(bool enumerable) {
  private_->enumerable = enumerable;
  private_->has_enumerable = true;
}

bool v8::PropertyDescriptor::enumerable() const {
  return private_->enumerable;
}

bool v8::PropertyDescriptor::has_enumerable() const {
  return private_->has_enumerable;
}

void v8::PropertyDescriptor::set_configurable(bool configurable) {
  private_->configurable = configurable;
  private_->has_configurable = true;
}

bool v8::PropertyDescriptor::configurable() const {
  return private_->configurable;
}

bool v8::PropertyDescriptor::has_configurable() const {
  return private_->has_configurable;
}

Maybe<bool> v8::Object::DefineOwnProperty(v8::Local<v8::Context> context,
                                          v8::Local<Name> key,
                                          v8::Local<Value> value,
                                          v8::PropertyAttribute attributes) {
  V8_CALL_TRACE();
  JerryValue* obj = reinterpret_cast<JerryValue*> (this);
  JerryValue* prop_name = reinterpret_cast<JerryValue*> (*key);
  JerryValue* prop_value = reinterpret_cast<JerryValue*> (*value);

  jerry_property_descriptor_t prop_desc = {
      .is_value_defined = true,
      .is_get_defined = false,
      .is_set_defined = false,
      .is_writable_defined = true,
      .is_writable = (bool)(attributes & ~PropertyAttribute::ReadOnly),
      .is_enumerable_defined = true,
      .is_enumerable = (bool)(attributes & ~PropertyAttribute::DontEnum),
      .is_configurable_defined = true,
      .is_configurable = (bool)(attributes & ~PropertyAttribute::DontDelete),
      .value = prop_value->value(),
      .getter = jerry_create_undefined(),
      .setter = jerry_create_undefined()
  };

  jerry_value_t result = jerry_define_own_property (obj->value(), prop_name->value(), &prop_desc);
  bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
  jerry_release_value(result);

  return Just(isOk);
}

Maybe<bool> v8::Object::DefineProperty(v8::Local<v8::Context> context,
                                       v8::Local<Name> key,
                                       PropertyDescriptor& descriptor) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);
  JerryValue* jname = reinterpret_cast<JerryValue*>(*key);

  jerry_property_descriptor_t prop_desc;
  jerry_init_property_descriptor_fields(&prop_desc);


  if (descriptor.has_value()) {
      prop_desc.is_value_defined = true;
      prop_desc.value = jerry_acquire_value(reinterpret_cast<JerryValue*>(*descriptor.value())->value());
  }
  if (descriptor.has_get()) {
      prop_desc.is_get_defined = true;
      prop_desc.getter = jerry_acquire_value(reinterpret_cast<JerryValue*>(*descriptor.get())->value());
  }
  if (descriptor.has_set()) {
      prop_desc.is_set_defined = true;
      prop_desc.setter = jerry_acquire_value(reinterpret_cast<JerryValue*>(*descriptor.set())->value());
  }
  if (descriptor.has_enumerable()) {
      prop_desc.is_enumerable_defined = true;
      prop_desc.is_enumerable = descriptor.enumerable();
  }
  if (descriptor.has_writable()) {
      prop_desc.is_writable_defined = true;
      prop_desc.is_writable = descriptor.writable();
  }
  if (descriptor.has_configurable()) {
      prop_desc.is_configurable_defined = true;
      prop_desc.is_configurable = descriptor.configurable();
  }

  jerry_value_t result = jerry_define_own_property(jobject->value(), jname->value(), &prop_desc);
  bool property_set = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
  jerry_release_value(result);

  jerry_free_property_descriptor_fields(&prop_desc);

  return Just(property_set);
}

Maybe<bool> v8::Object::SetPrivate(Local<Context> context, Local<Private> key,
                                   Local<Value> value) {
  V8_CALL_TRACE();
  return Just(reinterpret_cast<JerryValue*>(this)->SetPrivateProperty(
                  reinterpret_cast<JerryValue*>(*key),
                  reinterpret_cast<JerryValue*>(*value)));
}

MaybeLocal<Value> v8::Object::Get(Local<v8::Context> context,
                                  Local<Value> key) {
  V8_CALL_TRACE();
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  RETURN_HANDLE(Value, context->GetIsolate(), reinterpret_cast<JerryValue*> (this)->GetProperty(jkey));
}

MaybeLocal<Value> v8::Object::Get(Local<Context> context, uint32_t index) {
  V8_CALL_TRACE();
  RETURN_HANDLE(Value, context->GetIsolate(), reinterpret_cast<JerryValue*> (this)->GetPropertyIdx(index));
}

MaybeLocal<Value> v8::Object::GetPrivate(Local<Context> context,
                                         Local<Private> key) {
  V8_CALL_TRACE();
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  RETURN_HANDLE(Value, Isolate::GetCurrent(), reinterpret_cast<JerryValue*> (this)->GetPrivateProperty(jkey));
}

MaybeLocal<Value> v8::Object::GetOwnPropertyDescriptor(Local<Context> context,
                                                       Local<Name> key) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  JerryValue* property_object = jobject->GetOwnPropertyDescriptor(*jkey);
  RETURN_HANDLE(Value, context->GetIsolate(), property_object);
}

Local<Value> v8::Object::GetPrototype() {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

  JerryValue* jproto = new JerryValue(jerry_get_prototype(jobj->value()));
  RETURN_HANDLE(Value, JerryIsolate::toV8(JerryIsolate::GetCurrent()), jproto);
}

Maybe<bool> v8::Object::SetPrototype(Local<Context> context,
                                     Local<Value> value) {
  V8_CALL_TRACE();
  JerryValue* obj = reinterpret_cast<JerryValue*> (this);
  JerryValue* proto = reinterpret_cast<JerryValue*> (*value);

  jerry_value_t result = jerry_set_prototype (obj->value(), proto->value());
  bool isOk = !jerry_value_is_error(result) && jerry_get_boolean_value(result);
  jerry_release_value(result);

  return Just(isOk);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(Local<Context> context) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);

  JerryValue* names = jobject->GetPropertyNames();
  RETURN_HANDLE(Array, context->GetIsolate(), names);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(
    Local<Context> context, KeyCollectionMode mode,
    PropertyFilter property_filter, IndexFilter index_filter,
    KeyConversionMode key_conversion) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);

  JerryValue* names = jobject->GetAdvancedPropertyNames(mode, property_filter, index_filter, key_conversion);
  RETURN_HANDLE(Array, context->GetIsolate(), names);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);

  JerryValue* names = jobject->GetOwnPropertyNames();
  RETURN_HANDLE(Array, context->GetIsolate(), names);
}

Local<String> v8::Object::GetConstructorName() {
  UNIMPLEMENTED(4414);
  return Local<String>();
}

Maybe<bool> v8::Object::SetIntegrityLevel(Local<Context> context,
                                          IntegrityLevel level) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

  const char* prop_name = level == IntegrityLevel::kFrozen ? "freeze" : "seal";

  jerry_value_t prop_name_value = jerry_create_string ((const jerry_char_t *) prop_name);
  jerry_value_t result = JerryIsolate::fromV8(GetIsolate())->HelperSetIntegrityLevel().Call(jobj->value(), &prop_name_value, 1);
  jerry_release_value (prop_name_value);

  // [[TODO]] error handling?
  return Just(jerry_get_boolean_value(result));
}

Maybe<bool> v8::Object::Delete(Local<Context> context, Local<Value> key) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  return Just(jerry_delete_property(jobj->value(), jkey->value()));
}

Maybe<bool> v8::Object::DeletePrivate(Local<Context> context,
                                      Local<Private> key) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);
  return Just(jobj->DeleteInternalProperty(jkey));
}

Maybe<bool> v8::Object::Has(Local<Context> context, Local<Value> key) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
  JerryValue* jkey = reinterpret_cast<JerryValue*> (*key);

  jerry_value_t has_prop_js = jerry_has_property (jobj->value(), jkey->value());
  bool has_prop = jerry_get_boolean_value (has_prop_js);
  jerry_release_value (has_prop_js);

  return Just(has_prop);
}

Maybe<bool> v8::Object::HasPrivate(Local<Context> context, Local<Private> key) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);
  return Just(jobj->HasInternalProperty(jkey));
}

Maybe<bool> v8::Object::Delete(Local<Context> context, uint32_t index) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
  return Just(jerry_delete_property_by_index(jobj->value(), index));
}

Maybe<bool> v8::Object::Has(Local<Context> context, uint32_t index) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*> (this);
  jerry_value_t key_js = jerry_create_number((int32_t)index);

  jerry_value_t has_prop_js = jerry_has_property (jobj->value(), key_js);
  bool has_prop = jerry_get_boolean_value (has_prop_js);
  jerry_release_value (has_prop_js);
  jerry_release_value (key_js);

  // NOTE: Is `return Has(context, Local<Value>::New(context->GetIsolate(), JerryValue*))` better?
  return Just(has_prop);
}

Maybe<bool> Object::SetAccessor(Local<Context> context, Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                MaybeLocal<Value> data, AccessControl settings,
                                PropertyAttribute attribute,
                                SideEffectType getter_side_effect_type,
                                SideEffectType setter_side_effect_type) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

  JerryValue* jdata = NULL;
  if (!data.IsEmpty()) {
      Local<Value> dataValue;
      bool isOk = data.ToLocal(&dataValue);

      jdata = reinterpret_cast<JerryValue*>(*dataValue)->Copy();
  }

  AccessorEntry* entry = new AccessorEntry(reinterpret_cast<JerryValue*>(*name)->Copy(),
                                            getter,
                                            setter,
                                            jdata,
                                            settings,
                                            attribute);
  bool configured = JerryObjectTemplate::SetAccessor(jobj->value(), entry);
  JerryIsolate::fromV8(context->GetIsolate())->HiddenObjectTemplate()->SetAccessor(entry);

  return Just(configured);
}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context,
                                       Local<Name> key) {
  V8_CALL_TRACE();

  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  jerry_value_t has_prop = jerry_has_own_property(jobject->value(), jkey->value());
  bool property_exists = jerry_get_boolean_value(has_prop);
  jerry_release_value(property_exists);

  return Just(property_exists);
}

MaybeLocal<Value> v8::Object::GetRealNamedProperty(Local<Context> context,
                                                   Local<Name> key) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  jerry_value_t property = jerry_get_property(jobject->value(), jkey->value());

  JerryValue* result = NULL;
  if (!jerry_value_is_error(property)) {
      result = new JerryValue(property);
  } else {
      jerry_release_value(property);
  }

  RETURN_HANDLE(Value, context->GetIsolate(), result);
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(
    Local<Context> context, Local<Name> key) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  int attributes = PropertyAttribute::None;

  jerry_value_t target_object = jerry_acquire_value(jobject->value());
  bool found = false;
  do {
      jerry_property_descriptor_t prop_desc;
      jerry_init_property_descriptor_fields(&prop_desc);

      found = jerry_get_own_property_descriptor(target_object, jkey->value(), &prop_desc);

      if (found) {
          if (prop_desc.is_writable_defined && !prop_desc.is_writable) { attributes |= PropertyAttribute::ReadOnly; }
          if (prop_desc.is_enumerable_defined && !prop_desc.is_enumerable) { attributes |= PropertyAttribute::DontEnum; }
          if (prop_desc.is_configurable_defined && !prop_desc.is_configurable) { attributes |= PropertyAttribute::DontDelete; }
      } else {
          jerry_value_t new_target_object = jerry_get_prototype(target_object);
          jerry_release_value(target_object);

          target_object = new_target_object;
      }

      jerry_free_property_descriptor_fields(&prop_desc);
  } while (!found && !jerry_value_is_null(target_object));

  jerry_release_value(target_object);

  return found ? Just((PropertyAttribute)attributes) : Nothing<PropertyAttribute>();
}

Local<v8::Object> v8::Object::Clone() {
    V8_CALL_TRACE();
    // shallow copy!
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

    jerry_value_t arg = jobj->value();
    jerry_value_t result = JerryIsolate::fromV8(GetIsolate())->HelperObjectAssign().Call(jerry_create_undefined(), &arg, 1);

    RETURN_HANDLE(Object, GetIsolate(), new JerryValue(result));
}

Local<v8::Context> v8::Object::CreationContext() {
    V8_CALL_TRACE();
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);

    JerryValue* jctx = jobj->GetObjectCreationContext();

    // TODO: remove the hack:
    if (jctx == NULL) {
        jctx = JerryIsolate::GetCurrent()->CurrentContext();
    }

    // Copy the context
    RETURN_HANDLE(Context, GetIsolate(), jctx->Copy());
}

int v8::Object::GetIdentityHash() {
  V8_CALL_TRACE();
  return 0;
}

MaybeLocal<Function> Function::New(Local<Context> context,
                                   FunctionCallback callback, Local<Value> data,
                                   int length, ConstructorBehavior behavior,
                                   SideEffectType side_effect_type) {
  V8_CALL_TRACE();
  // TODO: maybe don't use function template?
  Local<FunctionTemplate> tmplt = FunctionTemplate::New(context->GetIsolate(), callback, data, Local<Signature>(), length, behavior);
  return tmplt->GetFunction(context);
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context, int argc,
                                         v8::Local<v8::Value> argv[]) const {
  V8_CALL_TRACE();
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

MaybeLocal<v8::Value> Function::Call(Local<Context> context,
                                     v8::Local<v8::Value> recv, int argc,
                                     v8::Local<v8::Value> argv[]) {
  V8_CALL_TRACE();
  const JerryValue* jfunc = reinterpret_cast<const JerryValue*>(this);
  const JerryValue* jthis = reinterpret_cast<const JerryValue*>(*recv);

  std::vector<jerry_value_t> arguments;
  arguments.resize(argc);
  for (int idx = 0; idx < argc; idx++) {
      arguments[idx] = reinterpret_cast<JerryValue*>(*argv[idx])->value();
  }

  jerry_value_t result = jerry_call_function(jfunc->value(), jthis->value(), &arguments[0], argc);
  JerryValue* return_value = JerryValue::TryCreateValue(JerryIsolate::GetCurrent(), result);

  if (return_value == NULL) {
      JerryIsolate::GetCurrent()->TryReportError();
  }

  RETURN_HANDLE(Value, Isolate::GetCurrent(), return_value);
}

void Function::SetName(v8::Local<v8::String> name) {
  V8_CALL_TRACE();
  // TODO: how to set the "name" of a function (in JS the function.name is readonly)
}

Local<Value> Function::GetDebugName() const {
  V8_CALL_TRACE();
  // TODO: only used by node_perf.cc
  return Local<Value>();
}

Local<v8::Value> Function::GetBoundFunction() const {
  V8_CALL_TRACE();
  // TODO: only used by node_perf.cc
  return Local<Value>();;
}

int Name::GetIdentityHash() {
  V8_CALL_TRACE();
  return 0;
}

int String::Length() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetStringLength();
}

int String::Utf8Length(Isolate* isolate) const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetStringUtf8Length();
}

int String::WriteUtf8(Isolate* v8_isolate, char* buffer, int capacity,
                      int* nchars_ref, int options) const {
  V8_CALL_TRACE();
  const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

  jerry_size_t bytes = jerry_string_to_utf8_char_buffer (jvalue->value(), (jerry_char_t *)buffer, capacity);

  if ((options & String::NO_NULL_TERMINATION) == 0) {
      buffer[bytes] = '\0';
  }

  return (int)bytes;
}

int String::WriteOneByte(Isolate* isolate, uint8_t* buffer, int start,
                         int length, int options) const {
  V8_CALL_TRACE();
  const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

  jerry_size_t str_length = jerry_get_string_length(jvalue->value());

  if ((length == -1) || ((jerry_size_t)(start + length) > str_length)) {
      length = str_length - start;
  }

  jerry_size_t bytes = jerry_substring_to_char_buffer (jvalue->value(), start, start + length, (jerry_char_t *)buffer, length);

  if ((options & String::NO_NULL_TERMINATION) == 0) {
      buffer[bytes] = '\0';
  }

  return (int)bytes;
}

int String::Write(Isolate* isolate, uint16_t* buffer, int start, int length,
                  int options) const {
  V8_CALL_TRACE();
  const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

  jerry_size_t str_length = jerry_get_utf8_string_length(jvalue->value());

  if ((length == -1) || ((jerry_size_t)(start + length) > str_length)) {
      length = str_length - start;
  }

  jerry_size_t bytes = jerry_substring_to_utf8_char_buffer (jvalue->value(), start, start + length, (jerry_char_t *)buffer, length);

  if ((options & String::NO_NULL_TERMINATION) == 0) {
      buffer[bytes] = '\0';
  }

  return (int)bytes;
}

bool v8::String::IsExternal() const {
  V8_CALL_TRACE ();
  const JerryString *jstring = reinterpret_cast<const JerryString*>(this);
  return (jstring->type() & JerryStringType::EXTERNAL) != 0;
}

bool v8::String::IsExternalOneByte() const {
  V8_CALL_TRACE ();
  const JerryString *jstring = reinterpret_cast<const JerryString*>(this);
  return (jstring->type() & (JerryStringType::EXTERNAL | JerryStringType::ONE_BYTE)) == (JerryStringType::EXTERNAL | JerryStringType::ONE_BYTE);
}

void v8::String::VerifyExternalStringResource(
    v8::String::ExternalStringResource* value) const {
  V8_CALL_TRACE ();
}

String::ExternalStringResource* String::GetExternalStringResourceSlow() const {
  V8_CALL_TRACE ();
  const JerryExternalString *jstring = reinterpret_cast<const JerryExternalString*>(this);
  return reinterpret_cast<ExternalStringResource*>(jstring->resource());
}

const v8::String::ExternalOneByteStringResource*
v8::String::GetExternalOneByteStringResource() const {
  V8_CALL_TRACE ();
  const JerryExternalString *jstring = reinterpret_cast<const JerryExternalString*>(this);
  return reinterpret_cast<ExternalOneByteStringResource*>(jstring->resource());
}

Local<Value> Symbol::Description() const {
  V8_CALL_TRACE();
  // TODO: create API function for it
  jerry_value_t symbol = reinterpret_cast<const JerryValue*>(this)->value();
  jerry_value_t symbol_to_object = jerry_value_to_object (symbol);
  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "description");

  jerry_value_t result = jerry_get_property (symbol_to_object, prop_name);
  jerry_release_value (prop_name);
  jerry_release_value (symbol_to_object);

  RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(result));
}

double Number::Value() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetNumberValue();
}

bool Boolean::Value() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetBooleanValue();
}

int64_t Integer::Value() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetInt64Value();
}

int32_t Int32::Value() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetInt32Value();
}

uint32_t Uint32::Value() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetUInt32Value();
}

int v8::Object::InternalFieldCount() {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  return jobj->InternalFieldCount();
}

Local<Value> v8::Object::SlowGetInternalField(int index) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  RETURN_HANDLE(Value, Isolate::GetCurrent(), jobj->GetInternalField<JerryValue*>(index));
}

void v8::Object::SetInternalField(int index, v8::Local<Value> value) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  // Indexing starts from 0!

  jobj->SetInternalField(index, reinterpret_cast<JerryValue*>(*value));
}

void* v8::Object::SlowGetAlignedPointerFromInternalField(int index) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  return jobj->GetInternalField<void*>(index);
}

void v8::Object::SetAlignedPointerInInternalField(int index, void* value) {
  V8_CALL_TRACE();
    JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
    // Indexing starts from 0!

    jobj->SetInternalField(index, value);
}

void v8::V8::InitializePlatform(Platform* platform) {
  V8_CALL_TRACE();
}

void v8::V8::ShutdownPlatform() {
  V8_CALL_TRACE();
}

bool v8::V8::Initialize(const int build_config) {
  V8_CALL_TRACE();
  return true;
}

bool TryHandleWebAssemblyTrapPosix(int sig_code, siginfo_t* info,
                                   void* context) {
  V8_CALL_TRACE();
  return false;
}

bool V8::EnableWebAssemblyTrapHandler(bool use_v8_signal_handler) {
  V8_CALL_TRACE();
  return false;
}

void v8::V8::SetEntropySource(EntropySource entropy_source) {
  V8_CALL_TRACE();
}

bool v8::V8::Dispose() {
  V8_CALL_TRACE();
  return true;
}

HeapStatistics::HeapStatistics()
    : total_heap_size_(0),
      total_heap_size_executable_(0),
      total_physical_size_(0),
      total_available_size_(0),
      used_heap_size_(0),
      heap_size_limit_(0),
      malloced_memory_(0),
      external_memory_(0),
      peak_malloced_memory_(0),
      does_zap_garbage_(false),
      number_of_native_contexts_(0),
      number_of_detached_contexts_(0),
      total_global_handles_size_(0),
      used_global_handles_size_(0) {
  UNIMPLEMENTED(5745);
}

HeapSpaceStatistics::HeapSpaceStatistics()
    : space_name_(nullptr),
      space_size_(0),
      space_used_size_(0),
      space_available_size_(0),
      physical_space_size_(0) {
  UNIMPLEMENTED(5761);
}

HeapCodeStatistics::HeapCodeStatistics()
    : code_and_metadata_size_(0),
      bytecode_and_metadata_size_(0),
      external_script_source_size_(0) {
  UNIMPLEMENTED(5774);
}

const char* V8::GetVersion() {
  V8_CALL_TRACE();
  return "JerryScript v2.4";
}

static std::string kContextSecurityTokenKey = "$$context_token";

Local<Context> v8::Context::New(
    v8::Isolate* external_isolate, v8::ExtensionConfiguration* extensions,
    v8::MaybeLocal<ObjectTemplate> global_template,
    v8::MaybeLocal<Value> global_object,
    DeserializeInternalFieldsCallback internal_fields_deserializer,
    v8::MicrotaskQueue* microtask_queue) {
  V8_CALL_TRACE();
  JerryValue* __handle = JerryValue::NewContextObject(JerryIsolate::fromV8(external_isolate));
  Local<Context> ctx = v8::Local<Context>::New(external_isolate, reinterpret_cast<Context*>(__handle));

  ctx->SetSecurityToken(ctx->Global().As<v8::Value>());

  return ctx;
}

MaybeLocal<Context> v8::Context::FromSnapshot(
    v8::Isolate* external_isolate, size_t context_snapshot_index,
    v8::DeserializeInternalFieldsCallback embedder_fields_deserializer,
    v8::ExtensionConfiguration* extensions, MaybeLocal<Value> global_object,
    v8::MicrotaskQueue* microtask_queue) {
  UNIMPLEMENTED(5988);
  return MaybeLocal<Context>();
}

void v8::Context::SetSecurityToken(Local<Value> token) {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);

  JerryValue name(jerry_create_string_from_utf8((const jerry_char_t*)kContextSecurityTokenKey.c_str()));
  ctx->SetInternalProperty(&name, reinterpret_cast<JerryValue*>(*token));
}

Local<Value> v8::Context::GetSecurityToken() {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  JerryValue name(jerry_create_string_from_utf8((const jerry_char_t*)kContextSecurityTokenKey.c_str()));
  JerryValue* prop = ctx->GetInternalProperty(&name);

  RETURN_HANDLE(Value, GetIsolate(), prop);
}

v8::Isolate* Context::GetIsolate() {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  assert(ctx->IsContextObject());
  return JerryIsolate::toV8(ctx->ContextGetIsolate());
}

v8::Local<v8::Object> Context::Global() {
  V8_CALL_TRACE();
  RETURN_HANDLE(Object, GetIsolate(), new JerryValue(jerry_get_global_object()));
}

static jerry_value_t trace_function(const jerry_value_t function_obj, const jerry_value_t this_val,
                                    const jerry_value_t args_p[], const jerry_length_t args_count) {
  return jerry_create_undefined();
}

Local<v8::Object> Context::GetExtrasBindingObject() {
  V8_CALL_TRACE();

  jerry_value_t object = jerry_create_object();
  jerry_value_t func = jerry_create_external_function(trace_function);

  jerry_value_t name = jerry_create_string((const jerry_char_t*)"isTraceCategoryEnabled");
  jerry_set_property (object, name, func);
  jerry_release_value (name);

  name = jerry_create_string((const jerry_char_t*)"trace");
  jerry_set_property (object, name, func);
  jerry_release_value (name);

  jerry_release_value (func);
  RETURN_HANDLE(Object, GetIsolate(), new JerryValue(object));
}

void Context::AllowCodeGenerationFromStrings(bool allow) {
  UNIMPLEMENTED(6082);
}

MaybeLocal<v8::Object> ObjectTemplate::NewInstance(Local<Context> context) {
  V8_CALL_TRACE();
  JerryObjectTemplate* object_template = reinterpret_cast<JerryObjectTemplate*>(this);

  // TODO: the function template's method should be set as the object's constructor
  JerryValue* new_instance = JerryValue::NewObject();
  object_template->InstallProperties(new_instance->value());

  if (object_template->HasProxyHandler()) {
    new_instance = object_template->Proxify(new_instance);
  }

  RETURN_HANDLE(Object, context->GetIsolate(), new_instance);
}

MaybeLocal<v8::Function> FunctionTemplate::GetFunction(Local<Context> context) {
  V8_CALL_TRACE();
  JerryFunctionTemplate* tmplt = reinterpret_cast<JerryFunctionTemplate*>(this);
  JerryValue *func = tmplt->GetFunction()->Copy();

  RETURN_HANDLE(Function, Isolate::GetCurrent(), func);
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
  V8_CALL_TRACE();
  JerryValue* object = reinterpret_cast<JerryValue*>(*value);

  if (!value->IsObject() || value->IsFunction()) {
      return false;
  }

  JerryV8FunctionHandlerData* data = JerryGetFunctionHandlerData(object->value());
  // TODO: the prototype chain should be traversed

  // TODO: do a better check not just a simple address check
  return data->function_template == reinterpret_cast<JerryFunctionTemplate*>(this);
}

Local<External> v8::External::New(Isolate* isolate, void* value) {
  UNIMPLEMENTED(6233);
  return v8::Local<v8::External>();
}

void* External::Value() const {
  UNIMPLEMENTED(6242);
  return NULL;
}

MaybeLocal<String> String::NewFromUtf8(Isolate* isolate, const char* data,
                                       NewStringType type, int length) {
  V8_CALL_TRACE();
  if (length >= String::kMaxLength) {
      return Local<String>();
  }

  if (length == -1) {
      length = strlen(data);
  }

  jerry_value_t str_value;

  if (!jerry_is_valid_utf8_string ((const jerry_char_t*)data, length)) {
    str_value = jerry_create_string_sz(NULL, 0);
  } else {
    str_value = jerry_create_string_sz_from_utf8((const jerry_char_t*)data, length);
  }

  RETURN_HANDLE(String, isolate, new JerryValue(str_value));
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate, const uint8_t* data,
                                          NewStringType type, int length) {
  V8_CALL_TRACE();
  if (length >= String::kMaxLength) {
      return Local<String>();
  }

  jerry_value_t str = JerryString::FromBuffer((const char*)data, length);

  RETURN_HANDLE(String, isolate, new JerryString(str));
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
                                          const uint16_t* data,
                                          NewStringType type, int length) {
  V8_CALL_TRACE();
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  std::string dest = convert.to_bytes(reinterpret_cast<const char16_t*>(data),
                                      reinterpret_cast<const char16_t*>(data + length));

  if (dest.size() >= String::kMaxLength) {
      return Local<String>();
  }

  jerry_value_t str = JerryString::FromBuffer(dest.c_str(), dest.size());

  RETURN_HANDLE(String, isolate, new JerryString(str, JerryStringType::TWO_BYTE));
}

Local<String> v8::String::Concat(Isolate* v8_isolate, Local<String> left,
                                 Local<String> right) {
  V8_CALL_TRACE();

  JerryValue *jleft = reinterpret_cast<JerryValue*>(*left);
  JerryValue *jright = reinterpret_cast<JerryValue*>(*right);

  jerry_value_t concat = jerry_binary_operation (JERRY_BIN_OP_ADD, jleft->value(), jright->value());

  RETURN_HANDLE (String, v8_isolate, new JerryValue (concat));
}

MaybeLocal<String> v8::String::NewExternalTwoByte(
    Isolate* isolate, v8::String::ExternalStringResource* resource) {
  V8_CALL_TRACE();

  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  std::string dest = convert.to_bytes(reinterpret_cast<const char16_t*>(resource->data()),
                                      reinterpret_cast<const char16_t*>(resource->data() + resource->length()));

  if (resource->length() >= String::kMaxLength) {
      return Local<String>();
  }

  jerry_value_t str = JerryString::FromBuffer(dest.c_str(), dest.size());

  RETURN_HANDLE(String, isolate, new JerryExternalString(str, reinterpret_cast<ExternalStringResourceBase*>(resource), JerryStringType::TWO_BYTE));
}

MaybeLocal<String> v8::String::NewExternalOneByte(
    Isolate* isolate, v8::String::ExternalOneByteStringResource* resource) {
  V8_CALL_TRACE();

  if (resource->length() >= String::kMaxLength) {
      return Local<String>();
  }

  jerry_value_t str = JerryString::FromBuffer((const char*)resource->data(), resource->length());

  RETURN_HANDLE(String, isolate, new JerryExternalString(str, reinterpret_cast<ExternalStringResourceBase*>(resource), JerryStringType::TWO_BYTE));
}

Isolate* v8::Object::GetIsolate() {
  V8_CALL_TRACE();
    return Isolate::GetCurrent();
}

Local<v8::Object> v8::Object::New(Isolate* isolate) {
  V8_CALL_TRACE();
  RETURN_HANDLE(Object, isolate, JerryValue::NewObject());
}

MaybeLocal<v8::Value> v8::Date::New(Local<Context> context, double time) {
  UNIMPLEMENTED(6650);
  return MaybeLocal<v8::Value>();
}

double v8::Date::ValueOf() const {
  UNIMPLEMENTED(6664);
  return 0;
}

Local<v8::Array> v8::Array::New(Isolate* isolate, int length) {
  V8_CALL_TRACE();
  if (length < 0) {
      length = 0;
  }

  jerry_value_t array_value = jerry_create_array(length);
  RETURN_HANDLE(Array, isolate, new JerryValue(array_value));
}

Local<v8::Array> v8::Array::New(Isolate* isolate, Local<Value>* elements,
                                size_t length) {
  V8_CALL_TRACE();

  jerry_value_t array_value = jerry_create_array(length);

  uint32_t length_limit = std::min<uint32_t>(UINT32_MAX, length);
  for (size_t i = 0; i < length_limit; i++) {
    JerryValue* arg = reinterpret_cast<JerryValue*>(*elements[i]);
    jerry_value_t result = jerry_set_property_by_index(array_value, i, arg->value());
  }

  RETURN_HANDLE(Array, isolate, new JerryValue(array_value));
}

uint32_t v8::Array::Length() const {
  V8_CALL_TRACE();
  const JerryValue* array = reinterpret_cast<const JerryValue*>(this);

  return jerry_get_array_length(array->value());
}

Local<v8::Map> v8::Map::New(Isolate* isolate) {
  V8_CALL_TRACE();
  jerry_value_t map = jerry_create_container (JERRY_CONTAINER_TYPE_MAP, NULL, 0);
  RETURN_HANDLE(Map, isolate, new JerryValue(map));
}

MaybeLocal<Map> Map::Set(Local<Context> context, Local<Value> key,
                         Local<Value> value) {
  V8_CALL_TRACE();
  JerryValue* jmap = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);
  JerryValue* jvalue = reinterpret_cast<JerryValue*>(*value);

  jerry_value_t args[] = { jmap->value(), jkey->value(), jvalue->value() };
  jerry_value_t result = JerryIsolate::fromV8(context->GetIsolate())->HelperMapSet().Call(jerry_create_undefined(), args, 3);
  jerry_release_value(result);

  return Local<Map>(this);
}

Local<v8::Set> v8::Set::New(Isolate* isolate) {
  jerry_value_t set = jerry_create_container (JERRY_CONTAINER_TYPE_SET, NULL, 0);
  RETURN_HANDLE(Set, isolate, new JerryValue(set));
}

MaybeLocal<Set> Set::Add(Local<Context> context, Local<Value> key) {
  V8_CALL_TRACE();
  JerryValue* jset = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  jerry_value_t args[] = { jset->value(), jkey->value(), };
  jerry_value_t result = JerryIsolate::fromV8(context->GetIsolate())->HelperSetAdd().Call(jerry_create_undefined(), args, 3);
  jerry_release_value(result);

  return Local<Set>(this);
}

MaybeLocal<Promise::Resolver> Promise::Resolver::New(Local<Context> context) {
  V8_CALL_TRACE();
  JerryValue* jpromise = JerryValue::NewPromise();

  RETURN_HANDLE(Promise::Resolver, context->GetIsolate(), jpromise);
}

Local<Promise> Promise::Resolver::GetPromise() {
  V8_CALL_TRACE();
  // TODO: maybe wrap the promise object into a resolver?
  RETURN_HANDLE(Promise, JerryIsolate::toV8(JerryIsolate::GetCurrent()), reinterpret_cast<JerryValue*>(this));
}

Maybe<bool> Promise::Resolver::Resolve(Local<Context> context,
                                       Local<Value> value) {
  V8_CALL_TRACE();
  JerryValue* jpromise = reinterpret_cast<JerryValue*>(this);
  JerryValue* jdata = reinterpret_cast<JerryValue*>(*value);
  jerry_value_t result = jerry_resolve_or_reject_promise(jpromise->value(), jdata->value(), true);

  if (jerry_value_is_error(result)) {
      return Just(false);
  } else {
      return Just(true);
  }
}

Maybe<bool> Promise::Resolver::Reject(Local<Context> context,
                                      Local<Value> value) {
  V8_CALL_TRACE();
  JerryValue* jpromise = reinterpret_cast<JerryValue*>(this);
  JerryValue* jdata = reinterpret_cast<JerryValue*>(*value);
  jerry_value_t result = jerry_resolve_or_reject_promise(jpromise->value(), jdata->value(), false);

  if (jerry_value_is_error(result)) {
      return Just(false);
  } else {
      return Just(true);
  }
}

Local<Value> Promise::Result() {
  V8_CALL_TRACE();
  JerryValue* jvalue = reinterpret_cast<JerryValue*>(this);

  JerryValue* jresult = JerryValue::TryCreateValue(JerryIsolate::GetCurrent(), jerry_get_promise_result(jvalue->value()));
  RETURN_HANDLE(Value, reinterpret_cast<Isolate*>(JerryIsolate::GetCurrent()), jresult);
}

Promise::PromiseState Promise::State() {
  V8_CALL_TRACE();
  JerryValue* jvalue = reinterpret_cast<JerryValue*>(this);

  switch(jerry_get_promise_state(jvalue->value())) {
      case JERRY_PROMISE_STATE_PENDING: return Promise::kPending;
      case JERRY_PROMISE_STATE_FULFILLED: return Promise::kFulfilled;
      case JERRY_PROMISE_STATE_REJECTED: return Promise::kRejected;
      default: break;
  }

  assert(false);
  return Promise::kPending;
}

Local<Value> Proxy::GetTarget() {
  UNIMPLEMENTED(7157);
  return Local<Value>();
}

Local<Value> Proxy::GetHandler() {
  UNIMPLEMENTED(7163);
  return Local<Value>();
}

class Utils {
public:
  static inline CompiledWasmModule Convert(
      std::shared_ptr<i::wasm::NativeModule> native_module) {
    return CompiledWasmModule{std::move(native_module)};
  }
};

CompiledWasmModule::CompiledWasmModule(std::shared_ptr<internal::wasm::NativeModule>) {
  V8_CALL_TRACE();
}

CompiledWasmModule WasmModuleObject::GetCompiledModule() {
  V8_CALL_TRACE();
  return Utils::Convert(NULL);
}

MaybeLocal<WasmModuleObject> WasmModuleObject::FromCompiledModule(
    Isolate* isolate, const CompiledWasmModule& compiled_module) {
  V8_CALL_TRACE();
  return MaybeLocal<WasmModuleObject>();
}

void* v8::ArrayBuffer::Allocator::Reallocate(void* data, size_t old_length,
                                             size_t new_length) {
  V8_CALL_TRACE();
  return this->Reallocate(data, old_length, new_length);
}

v8::ArrayBuffer::Allocator* v8::ArrayBuffer::Allocator::NewDefaultAllocator() {
  V8_CALL_TRACE();
  return JerryAllocator::toV8(JerryAllocator::NewDefaultAllocator());
}

bool v8::ArrayBuffer::IsDetachable() const {
  UNIMPLEMENTED(7265);
  return false;
}

/* ArrayBuffer & Allocator */

Local<ArrayBuffer> v8::ArrayBuffer::New(Isolate* isolate, size_t byte_length) {
  V8_CALL_TRACE();
  JerryBackingStore *backingStore = new JerryBackingStore(byte_length);

  RETURN_HANDLE(ArrayBuffer, isolate, JerryValue::NewArrayBuffer(backingStore));
}

Local<ArrayBuffer> v8::ArrayBuffer::New(
    Isolate* isolate, std::shared_ptr<BackingStore> backing_store) {
  V8_CALL_TRACE();

  RETURN_HANDLE(ArrayBuffer, isolate, JerryValue::NewArrayBuffer(JerryBackingStore::fromV8(backing_store.get())));
}

std::unique_ptr<v8::BackingStore> v8::ArrayBuffer::NewBackingStore(
    Isolate* isolate, size_t byte_length) {
  V8_CALL_TRACE();
  JerryBackingStore* jerryBackingStore = new JerryBackingStore(byte_length);
  return std::unique_ptr<v8::BackingStore>{reinterpret_cast<v8::BackingStore*>(new BackingStoreRef(jerryBackingStore))};
}

std::unique_ptr<v8::BackingStore> v8::ArrayBuffer::NewBackingStore(
    void* data, size_t byte_length, BackingStoreDeleterCallback deleter,
    void* deleter_data) {
  V8_CALL_TRACE();
  JerryBackingStore* jerryBackingStore = new JerryBackingStore(data, byte_length, deleter, deleter_data);
  return std::unique_ptr<v8::BackingStore>{reinterpret_cast<v8::BackingStore*>(new BackingStoreRef(jerryBackingStore))};
}

Local<ArrayBuffer> v8::ArrayBufferView::Buffer() {
  V8_CALL_TRACE();
  JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

  jerry_value_t buffer;

  if (jarray->IsTypedArray()) {
      buffer = jerry_get_typedarray_buffer (jarray->value(), NULL, NULL);
  } else if (jarray->IsDataView()) {
      buffer = jerry_get_dataview_buffer (jarray->value(), NULL, NULL);
  } else {
      abort();
  }

  RETURN_HANDLE(ArrayBuffer, Isolate::GetCurrent(), new JerryValue(buffer));
}

size_t v8::ArrayBufferView::CopyContents(void* dest, size_t byte_length) {
  UNIMPLEMENTED(7539);
  return 0;

  const JerryValue* jarray = reinterpret_cast<const JerryValue*> (this);
  jerry_value_t buffer;

  if (jarray->IsTypedArray()) {
      buffer = jerry_get_typedarray_buffer (jarray->value(), NULL, NULL);
  } else if (jarray->IsDataView()) {
      buffer = jerry_get_dataview_buffer (jarray->value(), NULL, NULL);
  } else {
      abort();
  }

  return jerry_arraybuffer_read (buffer, 0, (uint8_t*)dest, byte_length);
}

bool v8::ArrayBufferView::HasBuffer() const {
  V8_CALL_TRACE();
  const JerryValue* jarray = reinterpret_cast<const JerryValue*> (this);
  return jarray->IsTypedArray() || jarray->IsDataView();
}

size_t v8::ArrayBufferView::ByteOffset() {
  V8_CALL_TRACE();
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;
    jerry_length_t byteOffset = 0;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), &byteOffset, NULL);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), &byteOffset, NULL);
    } else {
        abort();
    }

    jerry_release_value(buffer);
    return byteOffset;
}

size_t v8::ArrayBufferView::ByteLength() {
  V8_CALL_TRACE();
    JerryValue* jarray = reinterpret_cast<JerryValue*> (this);

    jerry_value_t buffer;
    jerry_length_t byteLength = 0;

    if (jarray->IsTypedArray()) {
        buffer = jerry_get_typedarray_buffer (jarray->value(), NULL, &byteLength);
    } else if (jarray->IsDataView()) {
        buffer = jerry_get_dataview_buffer (jarray->value(), NULL, &byteLength);
    } else {
       abort();
    }

    jerry_release_value(buffer);

    return byteLength;
}

void v8::ArrayBuffer::Detach() {
  V8_CALL_TRACE();
  const JerryValue* jbuffer = reinterpret_cast<const JerryValue*> (this);
  jerry_detach_arraybuffer (jbuffer->value());
}

size_t v8::ArrayBuffer::ByteLength() const {
  V8_CALL_TRACE();
  const JerryValue* jbuffer = reinterpret_cast<const JerryValue*> (this);

  return (size_t) jerry_get_arraybuffer_byte_length (jbuffer->value());
}

size_t v8::TypedArray::Length() {
  V8_CALL_TRACE();
    return (size_t) jerry_get_typedarray_length (reinterpret_cast<JerryValue*> (this)->value());
}

Local<DataView> DataView::New(Local<ArrayBuffer> array_buffer,
                              size_t byte_offset, size_t byte_length) {
  V8_CALL_TRACE();
  JerryValue* abuffer = reinterpret_cast<JerryValue*> (*array_buffer);

  jerry_value_t dataview = jerry_create_dataview(abuffer->value(), byte_offset, byte_length);

  RETURN_HANDLE(DataView, JerryIsolate::toV8(JerryIsolate::GetCurrent()), new JerryValue(dataview));
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(
    Isolate* isolate, std::shared_ptr<BackingStore> backing_store) {
  V8_CALL_TRACE();
  JerryBackingStore *backingStore = JerryBackingStore::fromV8(backing_store.get());

  jerry_value_t arrayBuffer = jerry_create_arraybuffer_external (backingStore->byteLength(), (uint8_t*)backingStore->data(), NULL);
  jerry_value_t dataview = jerry_create_dataview(arrayBuffer, 0, backingStore->byteLength());
  jerry_release_value(arrayBuffer);

  RETURN_HANDLE(SharedArrayBuffer, JerryIsolate::toV8(JerryIsolate::GetCurrent()), new JerryValue(dataview));
}

Local<Symbol> v8::Symbol::New(Isolate* isolate, Local<String> name) {
  V8_CALL_TRACE();
  JerryValue* jname = reinterpret_cast<JerryValue*>(*name);

  jerry_value_t symbol_name = jerry_create_symbol (jname->value());
  RETURN_HANDLE(Symbol, isolate, new JerryValue(symbol_name));
}

Local<Private> v8::Private::New(Isolate* isolate, Local<String> name) {
  V8_CALL_TRACE();
  JerryValue* jname = reinterpret_cast<JerryValue*>(*name);
  jerry_value_t symbol_name = jerry_create_symbol (jname->value());

  RETURN_HANDLE(Private, isolate, new JerryValue(symbol_name));
}

Local<Private> v8::Private::ForApi(Isolate* isolate, Local<String> name) {
  V8_CALL_TRACE();
  JerryValue* jname = reinterpret_cast<JerryValue*>(*name);

  JerryValue *symbolHandle = JerryIsolate::fromV8(isolate)->GetGlobalSymbol(jname);
  RETURN_HANDLE(Private, isolate, symbolHandle);
}

Local<Number> v8::Number::New(Isolate* isolate, double value) {
  V8_CALL_TRACE();
  jerry_value_t result = jerry_create_number(value);
  RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

Local<Integer> v8::Integer::New(Isolate* isolate, int32_t value) {
  V8_CALL_TRACE();
  jerry_value_t result = jerry_create_number(value);
  RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

Local<Integer> v8::Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
  V8_CALL_TRACE();
  jerry_value_t result = jerry_create_number(value);
  RETURN_HANDLE(Integer, isolate, new JerryValue(result));
}

Local<BigInt> v8::BigInt::New(Isolate* isolate, int64_t value) {
  UNIMPLEMENTED(7950);
  return Local<BigInt>();
}

Local<BigInt> v8::BigInt::NewFromUnsigned(Isolate* isolate, uint64_t value) {
  UNIMPLEMENTED(7957);
  return Local<BigInt>();
}

MaybeLocal<BigInt> v8::BigInt::NewFromWords(Local<Context> context,
                                            int sign_bit, int word_count,
                                            const uint64_t* words) {
  UNIMPLEMENTED(7964);
  return MaybeLocal<BigInt>();
}

uint64_t v8::BigInt::Uint64Value(bool* lossless) const {
  UNIMPLEMENTED(7977);
  return 0;
}

int64_t v8::BigInt::Int64Value(bool* lossless) const {
  UNIMPLEMENTED(7982);
  return 0;
}

int BigInt::WordCount() const {
  UNIMPLEMENTED(7987);
  return 0;
}

void BigInt::ToWordsArray(int* sign_bit, int* word_count,
                          uint64_t* words) const {
  UNIMPLEMENTED(7992);
}

void Isolate::ReportExternalAllocationLimitReached() {
  V8_CALL_TRACE();
}

void Isolate::CheckMemoryPressure() {
  V8_CALL_TRACE();
}

HeapProfiler* Isolate::GetHeapProfiler() {
  V8_CALL_TRACE();
  return nullptr;
}

void Isolate::SetIdle(bool is_idle) {
  V8_CALL_TRACE();
}

ArrayBuffer::Allocator* Isolate::GetArrayBufferAllocator() {
  V8_CALL_TRACE();
  return JerryAllocator::toV8(new JerryAllocator());
}

bool Isolate::InContext() {
  V8_CALL_TRACE();
  return JerryIsolate::fromV8(this) == JerryIsolate::fromV8(this)->CurrentContext()->ContextGetIsolate();
}

void Isolate::ClearKeptObjects() {
  JerryIsolate::GetCurrent()->RunWeakCleanup();
}

v8::Local<v8::Context> Isolate::GetCurrentContext() {
  V8_CALL_TRACE();
  JerryValue* ctx = JerryIsolate::fromV8(this)->CurrentContext();

  RETURN_HANDLE(Context, this, ctx->Copy());
}

v8::Local<Value> Isolate::ThrowException(v8::Local<v8::Value> value) {
  V8_CALL_TRACE();
  JerryValue* jerror = reinterpret_cast<JerryValue*>(*value);
  JerryIsolate::fromV8(this)->SetError(jerry_create_error_from_value(jerror->value(), false));

  return value;
}

void Isolate::AddGCPrologueCallback(GCCallbackWithData callback, void* data,
                                    GCType gc_type) {
  V8_CALL_TRACE();
}

void Isolate::RemoveGCPrologueCallback(GCCallbackWithData callback,
                                       void* data) {
  V8_CALL_TRACE();
}

void Isolate::AddGCEpilogueCallback(GCCallbackWithData callback, void* data,
                                    GCType gc_type) {
  V8_CALL_TRACE();
}

void Isolate::RemoveGCEpilogueCallback(GCCallbackWithData callback,
                                       void* data) {
  V8_CALL_TRACE();
}

void Isolate::TerminateExecution() {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->Terminate();
}

void Isolate::CancelTerminateExecution() {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->CancelTerminate();
}

void Isolate::RequestInterrupt(InterruptCallback callback, void* data) {
  UNIMPLEMENTED(8162);
}

void Isolate::RequestGarbageCollectionForTesting(GarbageCollectionType type) {
  V8_CALL_TRACE();
  JerryIsolate::GetCurrent()->RunWeakCleanup();
}

Isolate* Isolate::GetCurrent() {
  V8_CALL_TRACE();
  return JerryIsolate::toV8(JerryIsolate::GetCurrent());
}

Isolate* Isolate::Allocate() {
  V8_CALL_TRACE();
  return JerryIsolate::toV8(new JerryIsolate());
}

void Isolate::Initialize(Isolate* isolate,
                         const v8::Isolate::CreateParams& params) {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(isolate)->InitializeJerryIsolate(params);
}

Isolate* Isolate::New(const Isolate::CreateParams& params) {
  V8_CALL_TRACE();
  return JerryIsolate::toV8(new JerryIsolate(params));
}

void Isolate::Dispose() {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->Dispose();
}

void Isolate::Enter() {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->Enter();
}

void Isolate::Exit() {
  V8_CALL_TRACE();
    JerryIsolate::fromV8(this)->Exit();
}

void Isolate::SetAbortOnUncaughtExceptionCallback(
    AbortOnUncaughtExceptionCallback callback) {
  V8_CALL_TRACE();
  // All uncaught exceptions will "terminate"
}

void Isolate::SetFatalErrorHandler(FatalErrorCallback that) {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->SetFatalErrorHandler(that);
}

void Isolate::SetHostImportModuleDynamicallyCallback(
    HostImportModuleDynamicallyCallback callback) {
  V8_CALL_TRACE();
}

void Isolate::SetHostInitializeImportMetaObjectCallback(
    HostInitializeImportMetaObjectCallback callback) {
  V8_CALL_TRACE();
}

void Isolate::SetPrepareStackTraceCallback(PrepareStackTraceCallback callback) {
  V8_CALL_TRACE();
  // No further specification
}

Isolate::DisallowJavascriptExecutionScope::DisallowJavascriptExecutionScope(
    Isolate* isolate,
    Isolate::DisallowJavascriptExecutionScope::OnFailure on_failure)
    : on_failure_(on_failure) {
  V8_CALL_TRACE();
}

Isolate::DisallowJavascriptExecutionScope::~DisallowJavascriptExecutionScope() {
 V8_CALL_TRACE();
}

Isolate::AllowJavascriptExecutionScope::AllowJavascriptExecutionScope(
    Isolate* isolate) {
  V8_CALL_TRACE();
}

Isolate::AllowJavascriptExecutionScope::~AllowJavascriptExecutionScope() {
  V8_CALL_TRACE();
}

i::Address* Isolate::GetDataFromSnapshotOnce(size_t index) {
  UNIMPLEMENTED(8435);
  return NULL;
}

void Isolate::GetHeapStatistics(HeapStatistics* heap_statistics) {
  UNIMPLEMENTED(8441);
}

size_t Isolate::NumberOfHeapSpaces() {
  UNIMPLEMENTED(8477);
  return 0;
}

bool Isolate::GetHeapSpaceStatistics(HeapSpaceStatistics* space_statistics,
                                     size_t index) {
  UNIMPLEMENTED(8481);
  return false;
}

bool Isolate::GetHeapCodeAndMetadataStatistics(
    HeapCodeStatistics* code_statistics) {
  UNIMPLEMENTED(8542);
  return false;
}

bool Isolate::MeasureMemory(std::unique_ptr<MeasureMemoryDelegate> delegate,
                            MeasureMemoryExecution execution) {
  UNIMPLEMENTED(8562);
  return false;
}

std::unique_ptr<MeasureMemoryDelegate> MeasureMemoryDelegate::Default(
    Isolate* isolate, Local<Context> context,
    Local<Promise::Resolver> promise_resolver, MeasureMemoryMode mode) {
  UNIMPLEMENTED(8568);
  return NULL;
}

void Isolate::SetAtomicsWaitCallback(AtomicsWaitCallback callback, void* data) {
  UNIMPLEMENTED(8633);
}

void Isolate::SetPromiseHook(PromiseHook hook) {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->SetPromiseHook(hook);
}

void Isolate::SetPromiseRejectCallback(PromiseRejectCallback callback) {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->SetPromiseRejectCallback(callback);
}

void Isolate::EnqueueMicrotask(Local<Function> v8_function) {
  UNIMPLEMENTED(8655);
}

void Isolate::SetMicrotasksPolicy(MicrotasksPolicy policy) {
  V8_CALL_TRACE();
}

void Isolate::LowMemoryNotification() {
  V8_CALL_TRACE();
}

void Isolate::SetStackLimit(uintptr_t stack_limit) {
  V8_CALL_TRACE();
}

void Isolate::AddNearHeapLimitCallback(v8::NearHeapLimitCallback callback,
                                       void* data) {
  V8_CALL_TRACE();
}

bool Isolate::AddMessageListenerWithErrorLevel(MessageCallback that,
                                               int message_levels,
                                               Local<Value> data) {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(this)->AddMessageListener(that);
  return true;
}

void Isolate::SetCaptureStackTraceForUncaughtExceptions(
    bool capture, int frame_limit, StackTrace::StackTraceOptions options) {
  V8_CALL_TRACE();
}

void v8::Isolate::DateTimeConfigurationChangeNotification(
    TimeZoneDetection time_zone_detection) {
  V8_CALL_TRACE();
}

void MicrotasksScope::PerformCheckpoint(Isolate* v8_isolate) {
  V8_CALL_TRACE();
}

String::Utf8Value::Utf8Value(v8::Isolate* isolate, v8::Local<v8::Value> obj)
    : str_(nullptr), length_(0) {
  V8_CALL_TRACE();
  JerryValue* jvalue = reinterpret_cast<JerryValue*>(*obj);

  if (jvalue == NULL || jvalue->value() == 0) {
      return;
  }

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
  V8_CALL_TRACE();
  delete [] str_;
}

String::Value::Value(v8::Isolate* isolate, v8::Local<v8::Value> obj)
    : str_(nullptr), length_(0) {
  V8_CALL_TRACE();
  V8_CALL_TRACE();
    JerryValue* jvalue = reinterpret_cast<JerryValue*>(*obj);

    if (jvalue == NULL || jvalue->value() == 0) {
        return;
    }

    jerry_value_t value;
    if (!jvalue->IsString()) {
        value = jerry_value_to_string(jvalue->value());
    } else {
        value = jvalue->value();
    }

    uint32_t size = (uint32_t)jerry_get_utf8_string_size(value);
    char* buffer = new char[size + 1];
    jerry_string_to_utf8_char_buffer (value, (jerry_char_t *)buffer, size + 1);
    buffer[size] = '\0';

    // Possible todo: remove the UTF8->UTF16 conversion.
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string* wstring = new std::u16string(converter.from_bytes(buffer));

    str_ = (uint16_t*) wstring->c_str();
    length_ = wstring->length();

    reinterpret_cast<JerryIsolate*>(isolate)->AddUTF16String(wstring);
    delete buffer;

    if (!jvalue->IsString()) {
        jerry_release_value(value);
    }
}

String::Value::~Value() {
  V8_CALL_TRACE();
  reinterpret_cast<JerryIsolate*>(Isolate::GetCurrent())->RemoveUTF16String(str_);
}


/* Exception & Error */
#define EXCEPTION_ERROR(error_class, error_type) \
    Local<Value> Exception::error_class(Local<String> raw_message) { \
        V8_CALL_TRACE(); \
        JerryValue* jstr = reinterpret_cast<JerryValue*>(*raw_message); \
        jerry_size_t req_sz = jerry_get_utf8_string_size(jstr->value()); \
        jerry_char_t str_buf_p[req_sz]; \
        jerry_string_to_utf8_char_buffer(jstr->value(), str_buf_p, req_sz); \
        jerry_value_t error_value = jerry_create_error_sz (error_type, str_buf_p, req_sz); \
        jerry_value_t error_object = jerry_get_value_from_error (error_value, true); \
        RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(error_object)); \
    }

EXCEPTION_ERROR(Error, JERRY_ERROR_COMMON);
EXCEPTION_ERROR(RangeError, JERRY_ERROR_RANGE);
EXCEPTION_ERROR(TypeError, JERRY_ERROR_TYPE);

Local<Message> Exception::CreateMessage(Isolate* isolate,
                                        Local<Value> exception) {
  V8_CALL_TRACE();
  JerryValue* jexception = reinterpret_cast<JerryValue*>(*exception);

  jerry_value_t to_string = jerry_value_to_string (jexception->value());

  JerryValue *message;

  if (jerry_value_is_error (to_string))
  {
    jerry_release_value (to_string);
    message = new JerryValue(jerry_create_string((jerry_char_t*) "Unknown error"));
  }
  else
  {
    message = new JerryValue(to_string);
  }

  RETURN_HANDLE(Message, isolate, message);
  //return Local<Message>();
}

v8::MaybeLocal<v8::Array> v8::Object::PreviewEntries(bool* is_key_value) {
  UNIMPLEMENTED(9826);
  return v8::MaybeLocal<v8::Array>();
}

void CpuProfiler::UseDetailedSourcePositionsForProfiling(Isolate* isolate) {
  V8_CALL_TRACE();
}

void HeapSnapshot::Delete() {
  V8_CALL_TRACE();
}

void HeapSnapshot::Serialize(OutputStream* stream,
                             HeapSnapshot::SerializationFormat format) const {
  V8_CALL_TRACE();
}

const HeapSnapshot* HeapProfiler::TakeHeapSnapshot(
    ActivityControl* control, ObjectNameResolver* resolver,
    bool treat_global_objects_as_roots) {
  V8_CALL_TRACE();
  return NULL;
}

void HeapProfiler::StartTrackingHeapObjects(bool track_allocations) {
  UNIMPLEMENTED(10687);
}

void HeapProfiler::AddBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  V8_CALL_TRACE();
}

void HeapProfiler::RemoveBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  V8_CALL_TRACE();
}

}
