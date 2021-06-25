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
#include "v8jerry_serialize.hpp"
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
do { \
    return (HANDLE)->NewLocal<T>(ISOLATE); \
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
  V8_CALL_TRACE();

  while (*str != '\0') {
      if (str[0] != '-' || str[1] != '-') {
          return;
      }

      str += 2;

      str = Flag::Update(str, true);

      if (str == NULL) {
          return;
      }

      while (*str == ' ' || *str == '\t') {
          str++;
      }
  }
}

void V8::SetFlagsFromString(const char* str, size_t length) {
  V8_CALL_TRACE();

  char* new_str = reinterpret_cast<char*>(malloc(length + 1));
  new_str[length] = '\0';
  SetFlagsFromString(new_str);
  free(new_str);
}

void V8::SetFlagsFromCommandLine(int* argc, char** argv, bool remove_flags) {
  V8_CALL_TRACE();

  int length = *argc;
  bool found = false;

  for (int idx = 0; idx < length; idx++) {
      const char* arg = argv[idx];

      if (arg[0] != '-' || arg[1] != '-') {
          /* Ignore arguments which does not start with '--' */
          continue;
      }
      arg += 2;

      if (Flag::Update(arg, false) == NULL) {
          continue;
      }

      /* At least one flag is found. */
      found = true;

      if (remove_flags) {
          argv[idx] = NULL;
      }
  }

  if (!remove_flags || !found) {
      return;
  }

  int dst = 0;

  for (int idx = 0; idx < length; idx++) {
      if (argv[idx] != NULL) {
          argv[dst++] = argv[idx];
      }
  }

  *argc = dst;
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
      case JerryHandle::LocalValue:
      case JerryHandle::PersistentValue:
      case JerryHandle::PersistentWeakValue:
          result = reinterpret_cast<JerryValue*>(jhandle)->CopyToGlobal();
          break;
      case JerryHandle::ObjectTemplate:
      case JerryHandle::FunctionTemplate:
          result = jhandle;
          break;
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
  JerryHandle* object = reinterpret_cast<JerryHandle*> (location);
  assert(object->type() == JerryHandle::PersistentValue
         || object->type() == JerryHandle::PersistentWeakValue
         || object->type() == JerryHandle::PersistentDeletedValue);
  return object->type() != JerryHandle::PersistentValue;
}

void V8::MakeWeak(i::Address* location, void* parameter,
                  WeakCallbackInfo<void>::Callback weak_callback,
                  WeakCallbackType type) {
  V8_CALL_TRACE();

  JerryValue* object = reinterpret_cast<JerryValue*>(location);

  assert(object->type() == JerryHandle::PersistentValue && type == WeakCallbackType::kParameter);

  object->MakeWeak(weak_callback, type, parameter);
}

static void PersistentWeakDeleteCallback(const WeakCallbackInfo<void>& data) {
  JerryValue** object_location = reinterpret_cast<JerryValue**>(data.GetParameter());

  JerryValue::DeleteValueWithoutRelease(*object_location);
  *object_location = NULL;
}

void V8::MakeWeak(i::Address** location_addr) {
  V8_CALL_TRACE();
  assert(!IsWeak(*location_addr));

  JerryValue* object = reinterpret_cast<JerryValue*> (*location_addr);
  void *parameter = reinterpret_cast<void*>(location_addr);

  object->MakeWeak(PersistentWeakDeleteCallback, WeakCallbackType::kParameter, parameter);
}

void* V8::ClearWeak(i::Address* location) {
  V8_CALL_TRACE();

  JerryValue* object = reinterpret_cast<JerryValue*> (location);

  if (object->type() != JerryHandle::PersistentWeakValue) {
      return NULL;
  }

  jerry_acquire_value(object->value());
  return object->ClearWeak();
}

void V8::DisposeGlobal(i::Address* location) {
  JerryHandle* jhandle = reinterpret_cast<JerryHandle*>(location);

  if (!JerryHandle::IsValueType(jhandle)) {
      return;
  }

  JerryValue* object = reinterpret_cast<JerryValue*> (location);

  if (object->type() == JerryHandle::PersistentDeletedValue) {
      JerryValue::DeleteValueWithoutRelease(object);
      return;
  }

  if (object->type() == JerryHandle::PersistentWeakValue) {
      object->ClearWeak();
      JerryValue::DeleteValueWithoutRelease(object);
      return;
  }

  delete object;
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
  JerryIsolate::fromV8(isolate_)->PushHandleScope(JerryHandleScopeType::Normal);
}

HandleScope::~HandleScope() {
  V8_CALL_TRACE();
  JerryIsolate::fromV8(isolate_)->PopHandleScope();
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
        case JerryHandle::PersistentValue:
        case JerryHandle::PersistentWeakValue: {
            // A persistent or global object should never be in a handle scope
            JerryValue* value = reinterpret_cast<JerryValue*>(jhandle)->Copy();
            iso->AddToHandleScope(value);
            return reinterpret_cast<internal::Address*>(value);
        }
        case JerryHandle::LocalValue:
            iso->AddToHandleScope(reinterpret_cast<JerryValue*>(jhandle));
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
    JerryIsolate::fromV8(isolate_)->PushHandleScope(JerryHandleScopeType::Sealed);
}

SealHandleScope::~SealHandleScope() {
    V8_CALL_TRACE();
    JerryIsolate::fromV8(isolate_)->PopHandleScope();
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
  RETURN_HANDLE(Value, GetIsolate(), ctx->GetInternalJerryValue(index));
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
    case JerryHandle::LocalValue:
    case JerryHandle::PersistentValue:
    case JerryHandle::PersistentWeakValue: {
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
  func->Inherit(parent);
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
  JerryFunctionTemplate* func = new JerryFunctionTemplate(behavior);
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
  V8_CALL_TRACE();
}

ScriptCompiler::CachedData::~CachedData() {
  V8_CALL_TRACE();
  if (buffer_policy == BufferOwned) {
    delete[] data;
  }
}

struct UnboundScriptData {
  Isolate* isolate;
  jerry_char_t *file_name;
  size_t file_name_length;
  jerry_char_t *source;
  size_t source_length;
};

void unboundScriptFreeCallback(void* native_p, jerry_object_native_info_t* info_p) {
  (void) info_p;

  UnboundScriptData* unboundScriptData = (UnboundScriptData*)native_p;

  if (unboundScriptData->file_name_length > 0) {
      delete[] unboundScriptData->file_name;
  }

  if (unboundScriptData->source_length > 0) {
      delete[] unboundScriptData->source;
  }

  delete unboundScriptData;
}

static jerry_object_native_info_t unboundScriptInfo {
  .free_cb = unboundScriptFreeCallback,
  .number_of_references = 0,
  .offset_of_references = 0,
};

Local<Script> UnboundScript::BindToCurrentContext() {
  V8_CALL_TRACE();
  const JerryValue* unboundScript = reinterpret_cast<const JerryValue*>(this);
  UnboundScriptData* unboundScriptData;
  jerry_get_object_native_pointer(unboundScript->value(), (void**)&unboundScriptData, &unboundScriptInfo);

  jerry_parse_options_t parse_options;
  parse_options.options = JERRY_PARSE_HAS_RESOURCE;
  parse_options.resource_name_p = unboundScriptData->file_name;
  parse_options.resource_name_length = unboundScriptData->file_name_length;

  jerry_value_t scriptFunction = jerry_parse(unboundScriptData->source,
                                             unboundScriptData->source_length,
                                             &parse_options);

  /* Should never happen (except out of memory) */
  if (V8_UNLIKELY(jerry_value_is_error(scriptFunction))) {
      jerry_release_value(scriptFunction);
      return Local<Script>();
  }

  RETURN_HANDLE(Script, unboundScriptData->isolate, new JerryValue(scriptFunction));
}

MaybeLocal<Value> Script::Run(Local<Context> context) {
  V8_CALL_TRACE();
  const JerryValue* script = reinterpret_cast<const JerryValue*>(this);
  Isolate* v8_isolate = context->GetIsolate();

  JerryIsolate::fromV8(v8_isolate)->IncTryDepth();
  jerry_value_t result = jerry_run(script->value());
  JerryIsolate::fromV8(v8_isolate)->DecTryDepth();

  RETURN_HANDLE(Value, v8_isolate, new JerryValue(result));
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

struct ModuleData {
  size_t requestsLength;
  jerry_value_t requests[];
};

static void ModuleDataFree(void* native_p, jerry_object_native_info_t* info_p) {
  (void) info_p;

  ModuleData* data = reinterpret_cast<ModuleData*>(native_p);

  size_t requestsLength = data->requestsLength;

  for (size_t i = 0; i < requestsLength; i++) {
      jerry_release_value(data->requests[i]);
  }

  free(data);
}

static jerry_object_native_info_t ModuleInfo = {
  .free_cb = ModuleDataFree,
  .number_of_references = 0,
  .offset_of_references = 0,
};

Module::Status Module::GetStatus() const {
  V8_CALL_TRACE();
  jerry_value_t module = reinterpret_cast<const JerryValue*>(this)->value();
  jerry_module_state_t state = jerry_module_get_state(module);

  switch (state) {
  case JERRY_MODULE_STATE_UNLINKED:
      return v8::Module::Status::kUninstantiated;
  case JERRY_MODULE_STATE_LINKING:
      return v8::Module::Status::kInstantiating;
  case JERRY_MODULE_STATE_LINKED:
      return v8::Module::Status::kInstantiated;
  case JERRY_MODULE_STATE_EVALUATING:
      return v8::Module::Status::kEvaluating;
  case JERRY_MODULE_STATE_EVALUATED:
      return v8::Module::Status::kEvaluated;
  default:
      return v8::Module::Status::kErrored;
  }
}

Local<Value> Module::GetException() const {
  V8_CALL_TRACE();
  const JerryValue* module = reinterpret_cast<const JerryValue*>(this);
  jerry_value_t result = JerryIsolate::GetModuleException(module->value());

  RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(result));
}

int Module::GetModuleRequestsLength() const {
  V8_CALL_TRACE();

  const JerryValue* module = reinterpret_cast<const JerryValue*>(this);
  ModuleData* data;

  if (jerry_get_object_native_pointer(module->value(), reinterpret_cast<void**>(&data), &ModuleInfo)) {
      return static_cast<int>(data->requestsLength);
  }

  return 0;
}

Local<String> Module::GetModuleRequest(int i) const {
  V8_CALL_TRACE();

  const JerryValue* module = reinterpret_cast<const JerryValue*>(this);
  ModuleData* data;

  if (jerry_get_object_native_pointer(module->value(), reinterpret_cast<void**>(&data), &ModuleInfo)) {
      if (i >= 0 && i < static_cast<int>(data->requestsLength)) {
          RETURN_HANDLE(String, Isolate::GetCurrent(), new JerryValue(jerry_acquire_value(data->requests[i])));
      }
  }

  return Local<String>();
}

Local<Value> Module::GetModuleNamespace() {
  V8_CALL_TRACE();
  jerry_value_t module = reinterpret_cast<const JerryValue*>(this)->value();
  jerry_value_t result = jerry_module_get_namespace(module);

  RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(result));
}

Local<UnboundModuleScript> Module::GetUnboundModuleScript() {
  V8_CALL_TRACE();
  RETURN_HANDLE(UnboundModuleScript, Isolate::GetCurrent(), new JerryValue(jerry_create_object()));
}

int Module::GetIdentityHash() const {
  V8_CALL_TRACE();
  const JerryValue* module = reinterpret_cast<const JerryValue*>(this);
  return (int)module->value();
}

struct InstantiateModuleCallbackData {
  v8::Module::ResolveCallback callback;
  v8::Isolate *isolate;
  JerryValue* context;
};

jerry_value_t InstantiateModuleCallback(const jerry_value_t jspecifier,
                                        const jerry_value_t jreferrer,
                                        void *user_p) {
  InstantiateModuleCallbackData* callbackData = reinterpret_cast<InstantiateModuleCallbackData*>(user_p);

  v8::HandleScope handle_scope(callbackData->isolate);

  JerryValueNoRelease specifier(jspecifier);
  JerryValueNoRelease referrer(jreferrer);

  Local<Module> module;
  if (callbackData->callback(callbackData->context->AsLocal<v8::Context>(),
                             specifier.AsLocal<v8::String>(),
                             referrer.AsLocal<v8::Module>()).ToLocal(&module)) {
      return jerry_acquire_value (reinterpret_cast<JerryValue*>(*module)->value());
  }

  JerryIsolate* isolate = JerryIsolate::fromV8(callbackData->isolate);

  if (isolate->HasError()) {
      return jerry_create_error_from_value(isolate->TakeError(), true);
  }

  return jerry_create_error(JERRY_ERROR_RANGE, reinterpret_cast<const jerry_char_t *>("No module instantiated"));
}

Maybe<bool> Module::InstantiateModule(Local<Context> context,
                                      Module::ResolveCallback callback) {
  V8_CALL_TRACE();

  const JerryValue* module = reinterpret_cast<const JerryValue*>(this);
  InstantiateModuleCallbackData callbackData;
  callbackData.callback = callback;
  callbackData.isolate = Isolate::GetCurrent();
  callbackData.context = reinterpret_cast<JerryValue*>(*context);

  jerry_value_t result = jerry_module_link(module->value(), InstantiateModuleCallback, reinterpret_cast<void*>(&callbackData));

  if (!jerry_value_is_error(result))
  {
      jerry_release_value(result);
      return Just(true);
  }

  JerryIsolate::fromV8(context->GetIsolate())->SetError(result);
  return Nothing<bool>();
}

MaybeLocal<Value> Module::Evaluate(Local<Context> context) {
  V8_CALL_TRACE();

  jerry_value_t module = reinterpret_cast<const JerryValue*>(this)->value();

  jerry_module_state_t state = jerry_module_get_state(module);

  if (state == JERRY_MODULE_STATE_EVALUATING || state == JERRY_MODULE_STATE_EVALUATED)
  {
      RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(jerry_create_undefined()));
  }

  jerry_value_t result;
  if (state == JERRY_MODULE_STATE_ERROR)
  {
      result = jerry_create_error_from_value (JerryIsolate::GetModuleException(module), true);
  }
  else
  {
      jerry_value_t prev_realm = jerry_set_realm(reinterpret_cast<JerryValue*>(*context)->value());
      result = jerry_module_evaluate(module);
      jerry_set_realm(prev_realm);

      if (!jerry_value_is_error(result)) {
          RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(result));
      }
  }

  JerryIsolate::fromV8(context->GetIsolate())->SetError(result);
  return MaybeLocal<Value>();
}

struct SyntheticModuleData {
  jerry_value_t context;
  v8::Isolate* v8isolate;
  v8::Module::SyntheticModuleEvaluationSteps evaluation_steps;
};

void syntheticModuleFreeCallback(void* native_p, jerry_object_native_info_t* info_p) {
  SyntheticModuleData *syntheticModuleData = reinterpret_cast<SyntheticModuleData*>(native_p);

  jerry_native_pointer_release_references (native_p, info_p);
  delete syntheticModuleData;
}

static jerry_object_native_info_t SyntheticModuleInfo {
  .free_cb = syntheticModuleFreeCallback,
  .number_of_references = 1,
  .offset_of_references = 0,
};

jerry_value_t native_module_evaluate (const jerry_value_t native_module) {
  SyntheticModuleData* data;

  jerry_get_object_native_pointer(native_module, (void**)&data, &SyntheticModuleInfo);
  jerry_delete_object_native_pointer (native_module, &SyntheticModuleInfo);

  v8::HandleScope handle_scope(data->v8isolate);
  v8::TryCatch try_catch(data->v8isolate);

  JerryValueNoRelease context(data->context);
  JerryValueNoRelease module(native_module);

  syntheticModuleFreeCallback(reinterpret_cast<void*>(data), &SyntheticModuleInfo);

  v8::Local<v8::Value> resultValue;

  if (data->evaluation_steps(context.AsLocal<v8::Context>(), module.AsLocal<v8::Module>()).ToLocal(&resultValue)) {
    return jerry_acquire_value(reinterpret_cast<JerryValue*>(*resultValue)->value());
  }

  jerry_value_t result;

  if (try_catch.HasCaught()) {
    jerry_value_t error = reinterpret_cast<JerryValue*>(*try_catch.Exception())->value();
    result = jerry_create_error_from_value (error, false);
  } else {
    result = jerry_create_error (JERRY_ERROR_RANGE, (const jerry_char_t *) "No value returned");
  }

  return result;
}

Local<Module> Module::CreateSyntheticModule(
    Isolate* isolate, Local<String> module_name,
    const std::vector<Local<v8::String>>& export_names,
    v8::Module::SyntheticModuleEvaluationSteps evaluation_steps) {
  V8_CALL_TRACE();

  size_t size = export_names.size();

  jerry_value_t *exports = (jerry_value_t *) malloc (size * sizeof (jerry_value_t));

  for (size_t i = 0; i < size; i++) {
    exports[i] = reinterpret_cast<JerryValue*>(*(export_names[i]))->value();
  }

  jerry_value_t result = jerry_native_module_create (native_module_evaluate, exports, size);
  free (exports);

  SyntheticModuleData *data = new SyntheticModuleData();

  jerry_native_pointer_init_references (reinterpret_cast<void*>(data), &SyntheticModuleInfo);
  data->v8isolate = isolate;
  data->evaluation_steps = evaluation_steps;

  jerry_set_object_native_pointer(result, reinterpret_cast<void*>(data), &SyntheticModuleInfo);

  jerry_native_pointer_set_reference (&data->context, JerryIsolate::fromV8(isolate)->CurrentContext()->value());

  RETURN_HANDLE(Module, isolate, new JerryValue(result));
}

Maybe<bool> Module::SetSyntheticModuleExport(Isolate* isolate,
                                             Local<String> export_name,
                                             Local<Value> export_value) {
  JerryValue* module = reinterpret_cast<JerryValue*>(this);
  JerryValue* name = reinterpret_cast<JerryValue*>(*export_name);
  JerryValue* value = reinterpret_cast<JerryValue*>(*export_value);

  jerry_value_t result = jerry_native_module_set_export(module->value(), name->value(), value->value());

  if (jerry_value_is_error(result)) {
    JerryIsolate::GetCurrent()->SetError(result);
    return Nothing<bool>();
  }

  return Just(true);
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundScript(
    Isolate* v8_isolate, Source* source, CompileOptions options,
    NoCacheReason no_cache_reason) {
  V8_CALL_TRACE();

  Local<String> file;

  if (source->resource_name.IsEmpty()) {
      file = source->resource_name.As<String>();
  } else {
      bool isOk =source->resource_name->ToString(v8_isolate->GetCurrentContext()).ToLocal(&file);

      if (!isOk) {
        return MaybeLocal<UnboundScript>();
      }
  }

  String::Utf8Value file_name(v8_isolate, file);
  String::Utf8Value source_string(v8_isolate, source->source_string);

  if (options == CompileOptions::kConsumeCodeCache) {
      CachedData* data = source->cached_data;
      if (data->length != source_string.length() || memcmp(data->data,  reinterpret_cast<void*>(*source_string), data->length) != 0) {
          data->rejected = true;
      }
  }

  size_t file_name_length = (size_t)file_name.length();
  size_t source_string_length = (size_t)source_string.length();

  jerry_parse_options_t parse_options;
  parse_options.options = JERRY_PARSE_HAS_RESOURCE | JERRY_PARSE_HAS_START;
  parse_options.resource_name_p = (const jerry_char_t*)*file_name;
  parse_options.resource_name_length = file_name_length;
  parse_options.start_line = static_cast<uint32_t>(source->resource_line_offset->Value() + 1);
  parse_options.start_column = static_cast<uint32_t>(source->resource_column_offset->Value() + 1);

  /* Compiling for checking syntax errors. */
  jerry_value_t scriptFunction = jerry_parse((const jerry_char_t*)*source_string,
                                             source_string_length,
                                             &parse_options);

  if (V8_UNLIKELY(jerry_value_is_error(scriptFunction))) {
      JerryIsolate::fromV8(v8_isolate)->SetError(scriptFunction);
      return MaybeLocal<UnboundScript>();
  }

  jerry_release_value(scriptFunction);

  UnboundScriptData *unboundScriptData = new UnboundScriptData;
  unboundScriptData->isolate = v8_isolate;

  unboundScriptData->file_name_length = file_name_length;
  if (file_name_length > 0) {
      unboundScriptData->file_name = new jerry_char_t[file_name_length];
      memcpy(unboundScriptData->file_name, *file_name, file_name_length);
  } else {
      unboundScriptData->file_name = NULL;
  }

  unboundScriptData->source_length = source_string_length;
  if (source_string_length > 0) {
      unboundScriptData->source = new jerry_char_t[source_string_length];
      memcpy(unboundScriptData->source, *source_string, source_string_length);
  } else {
      unboundScriptData->source = reinterpret_cast<jerry_char_t*>(1);
  }

  jerry_value_t result = jerry_create_object();
  jerry_set_object_native_pointer(result, unboundScriptData, &unboundScriptInfo);

  if (options != CompileOptions::kConsumeCodeCache) {
      source->cached_data = new CachedData(unboundScriptData->source, unboundScriptData->source_length);
  }

  RETURN_HANDLE(UnboundScript, v8_isolate, new JerryValue(result));
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
  String::Utf8Value file_name(isolate, file);

  jerry_parse_options_t parse_options;
  parse_options.options = JERRY_PARSE_MODULE | JERRY_PARSE_HAS_RESOURCE | JERRY_PARSE_HAS_START;
  parse_options.resource_name_p = (const jerry_char_t*)*file_name;
  parse_options.resource_name_length = (size_t)file_name.length();
  parse_options.start_line = static_cast<uint32_t>(source->resource_line_offset->Value() + 1);
  parse_options.start_column = static_cast<uint32_t>(source->resource_column_offset->Value() + 1);

  jerry_value_t module = jerry_parse((const jerry_char_t*)*text, text.length(), &parse_options);

  if (jerry_value_is_error(module)) {
      JerryIsolate::fromV8(isolate)->SetError(module);
      return MaybeLocal<Module>();
  }

  size_t requestsLength = jerry_module_get_number_of_requests(module);
  size_t size = sizeof(ModuleData) + requestsLength * sizeof(jerry_value_t);

  ModuleData* data = reinterpret_cast<ModuleData*>(malloc(size));
  data->requestsLength = requestsLength;

  for (size_t i = 0; i < requestsLength; i++) {
      data->requests[i] = jerry_module_get_request(module, i);
  }

  jerry_set_object_native_pointer(module, data, &ModuleInfo);

  RETURN_HANDLE(Module, isolate, new JerryValue(module));
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
    args.append((const char*)*arg, (size_t) arg.length());
    if (i != arguments_count - 1) {
      args.append(",", 1);
    }
  }

  String::Utf8Value source_string(isolate, source->source_string);
  String::Utf8Value fileName(isolate, file);

  const char* source_raw_string = *source_string;
  int source_raw_length = source_string.length();

  if (source_raw_length >= 1 && *source_raw_string == '#') {
    // Cut she-bang
    while (source_raw_length > 0 && *source_raw_string != '\r' && *source_raw_string != '\n') {
      source_raw_length--;
      source_raw_string++;
    }
  }

  jerry_parse_options_t parse_options;
  parse_options.options = JERRY_PARSE_HAS_RESOURCE | JERRY_PARSE_HAS_START;
  parse_options.resource_name_p = (const jerry_char_t*)*fileName;
  parse_options.resource_name_length = fileName.length();
  parse_options.start_line = static_cast<uint32_t>(source->resource_line_offset->Value() + 1);
  parse_options.start_column = static_cast<uint32_t>(source->resource_column_offset->Value() + 1);

  jerry_value_t scriptFunction = jerry_parse_function((const jerry_char_t*) args.c_str(),
                                                      args.length(),
                                                      (const jerry_char_t*)source_raw_string,
                                                      source_raw_length,
                                                      &parse_options);

  if (script_or_module_out != nullptr) {
    jerry_value_t object = jerry_create_object();
    JerryValue* script_or_module = JerryValue::TryCreateValue(isolate, object);
    *script_or_module_out = script_or_module->NewLocal<ScriptOrModule>(isolate);
  }

  JerryValue* result = JerryValue::TryCreateValue(isolate, scriptFunction);
  RETURN_HANDLE(Function, isolate, result);
}

uint32_t ScriptCompiler::CachedDataVersionTag() {
  V8_CALL_TRACE();
  return 0;
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundScript> unbound_script) {
  V8_CALL_TRACE();

  JerryValue* script = reinterpret_cast<JerryValue*>(*unbound_script);

  UnboundScriptData* unboundScriptData;
  jerry_get_object_native_pointer(script->value(), (void**)&unboundScriptData, &unboundScriptInfo);

  size_t source_length = unboundScriptData->source_length;
  uint8_t* data = NULL;
  if (source_length > 0) {
      data = new jerry_char_t[source_length];
      memcpy(data, unboundScriptData->source, source_length);
  }

  return new CachedData(data, source_length, CachedData::BufferOwned);
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
      is_verbose_(false),
      can_continue_(true),
      capture_message_(true),
      rethrow_(false),
      has_terminated_(false),
      exception_(NULL),
      message_obj_(NULL) {
  V8_CALL_TRACE();

  JerryIsolate* jisolate = JerryIsolate::fromV8(isolate_);

  next_ = jisolate->GetLastTryCatch();
  jisolate->SetLastTryCatch(this);

  if (jisolate->HasError()) {
      if (next_ == NULL) {
          jisolate->SetGlobalError(new JerryValue(jisolate->TakeError()));
          return;
      }

      if (next_->exception_ != NULL) {
          delete reinterpret_cast<JerryValue*>(next_->exception_);
      }
      next_->exception_ = new JerryValue(jisolate->TakeError());
  }
}

v8::TryCatch::~TryCatch() {
  V8_CALL_TRACE();
  JerryIsolate* isolate = JerryIsolate::fromV8(isolate_);

  isolate->SetLastTryCatch(next_);

  if (isolate->HasError()) {
      if (rethrow_) {
          if (next_ == NULL) {
              if (isolate->GetGlobalError()) {
                  delete isolate->GetGlobalError();
                  isolate->SetGlobalError(NULL);
              }

              isolate->ProcessError(false);
              return;
          }

          if (next_->exception_ != NULL) {
              delete reinterpret_cast<JerryValue*>(next_->exception_);
              next_->exception_ = NULL;
          }
          return;
      }

      if (is_verbose_) {
          isolate->ProcessError(true);
          return;
      }
  }

  if (next_ == NULL) {
      isolate->RestoreError(isolate->TakeGlobalError());
  } else {
      isolate->RestoreError(reinterpret_cast<JerryValue*>(next_->exception_));
  }
}

bool v8::TryCatch::HasCaught() const {
  V8_CALL_TRACE();

  JerryIsolate* isolate = JerryIsolate::fromV8(isolate_);

  if (isolate->GetLastTryCatch() != this) {
      return exception_ != NULL;
  }

  return isolate->HasError();
}

bool v8::TryCatch::CanContinue() const {
  V8_CALL_TRACE();
  return can_continue_;
}

bool v8::TryCatch::HasTerminated() const {
  V8_CALL_TRACE();
  return has_terminated_;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  V8_CALL_TRACE();
  rethrow_ = true;

  // TODO: return what?
  return Local<Value>();
}

v8::Local<Value> v8::TryCatch::Exception() const {
  V8_CALL_TRACE();

  JerryIsolate* isolate = JerryIsolate::fromV8(isolate_);
  jerry_value_t exception;

  if (isolate->GetLastTryCatch() != this) {
      if (exception_ == NULL ) {
          return Local<Value>();
      }

      exception = reinterpret_cast<JerryValue*>(exception_)->value();
  } else {
      if (!isolate->HasError()) {
          return Local<Value>();
      }
      exception = isolate->GetError();
  }

  RETURN_HANDLE(Value, reinterpret_cast<v8::Isolate*>(isolate_), new JerryValue(jerry_acquire_value(exception)));
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
    new JerryValue(jerry_create_string((const jerry_char_t*)"")),
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

  v8::Isolate* isolate = GetIsolate();
  return ScriptOrigin(arr[0]->NewLocal<Value>(isolate),
                      arr[1]->NewLocal<Integer>(isolate),
                      arr[2]->NewLocal<Integer>(isolate),
                      arr[3]->NewLocal<Boolean>(isolate),
                      arr[4]->NewLocal<Integer>(isolate),
                      arr[5]->NewLocal<Value>(isolate),
                      arr[6]->NewLocal<Boolean>(isolate),
                      arr[7]->NewLocal<Boolean>(isolate),
                      arr[8]->NewLocal<Boolean>(isolate),
                      arr[9]->NewLocal<PrimitiveArray>(isolate));
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

struct ValueSerializer::PrivateData {
  explicit PrivateData(i::Isolate* i, ValueSerializer::Delegate* delegate)
      : isolate(i), serializer(i, delegate) {}
  i::Isolate* isolate;
  JerrySerialize::ValueSerializer serializer;
};

ValueSerializer::ValueSerializer(Isolate* isolate, Delegate* delegate)
    : private_(new PrivateData(reinterpret_cast<i::Isolate*>(isolate), delegate)) {
}

ValueSerializer::~ValueSerializer() {
  delete private_;
}

void ValueSerializer::WriteHeader() {
  private_->serializer.WriteHeader();
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {
  private_->serializer.SetTreatArrayBufferViewsAsHostObjects(mode);
}

Maybe<bool> ValueSerializer::WriteValue(Local<Context> context,
                                        Local<Value> value) {
  bool result = private_->serializer.WriteValue(reinterpret_cast<JerryValue*>(*value));
  return Just(result);
}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
  auto result = private_->serializer.Release();
  return result;
}

void ValueSerializer::TransferArrayBuffer(uint32_t transfer_id,
                                          Local<ArrayBuffer> array_buffer) {
  UNIMPLEMENTED(3191);
}

void ValueSerializer::WriteUint32(uint32_t value) {
  private_->serializer.WriteUint32(value);
}

void ValueSerializer::WriteUint64(uint64_t value) {
  private_->serializer.WriteUint64(value);
}

void ValueSerializer::WriteDouble(double value) {
  private_->serializer.WriteDouble(value);
}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
  private_->serializer.WriteRawBytes(source, length);
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

struct ValueDeserializer::PrivateData {
  PrivateData(i::Isolate* i,
              const uint8_t* data,
              const size_t size,
              ValueDeserializer::Delegate* delegate)
      : isolate(i), deserializer(i, data, size, delegate) {}
  i::Isolate* isolate;
  JerrySerialize::ValueDeserializer deserializer;
};

ValueDeserializer::ValueDeserializer(Isolate* isolate,
                                     const uint8_t* data,
                                     size_t size,
                                     Delegate* delegate)
    : private_(
          new ValueDeserializer::PrivateData(reinterpret_cast<i::Isolate*>(isolate), data, size, delegate)) {}

ValueDeserializer::~ValueDeserializer() {
  delete private_;
}

Maybe<bool> ValueDeserializer::ReadHeader(Local<Context> context) {
  bool result = private_->deserializer.ReadHeader();
  return Just(result);
}

uint32_t ValueDeserializer::GetWireFormatVersion() const {
  UNIMPLEMENTED(3305);
  return 0;
}

MaybeLocal<Value> ValueDeserializer::ReadValue(Local<Context> context) {
  JerryValue* result = private_->deserializer.ReadValue();
  RETURN_HANDLE(Value, context->GetIsolate(), result);
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
  return private_->deserializer.ReadUint32(value);
}

bool ValueDeserializer::ReadUint64(uint64_t* value) {
  return private_->deserializer.ReadUint64(value);
}

bool ValueDeserializer::ReadDouble(double* value) {
  return private_->deserializer.ReadDouble(value);
}

bool ValueDeserializer::ReadRawBytes(size_t length, const void** data) {
  return private_->deserializer.ReadRawBytes(length, data);
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
  Isolate* isolate = context->GetIsolate();
  RETURN_HANDLE(String, isolate, reinterpret_cast<const JerryValue*>(this)->ToString(isolate));
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
  V8_CALL_TRACE();

  jerry_value_t jvalue = reinterpret_cast<const JerryValue*>(this)->value();
  jerry_value_t result;

  if (jerry_value_is_symbol(jvalue)) {
      result = jerry_get_symbol_descriptive_string(jvalue);
  } else {
      result = jerry_value_to_string(jvalue);
  }

  /* Never throws any exception. */
  if (V8_UNLIKELY(jerry_value_is_error(result))) {
      jerry_release_value(result);
      return Local<String>();
  }

  RETURN_HANDLE(String, context->GetIsolate(), new JerryValue(result));
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
  V8_CALL_TRACE();
  Isolate* isolate = context->GetIsolate();
  RETURN_HANDLE(Object, isolate, reinterpret_cast<const JerryValue*>(this)->ToObject(isolate));
}

bool Value::BooleanValue(Isolate* v8_isolate) const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->BooleanValue();
}

Local<Boolean> Value::ToBoolean(Isolate* v8_isolate) const {
  V8_CALL_TRACE();
  RETURN_HANDLE(Boolean, v8_isolate, reinterpret_cast<const JerryValue*>(this)->ToBoolean());
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
  V8_CALL_TRACE();
  Isolate* isolate = context->GetIsolate();
  RETURN_HANDLE(Number, isolate, reinterpret_cast<const JerryValue*>(this)->ToNumber(isolate));
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
  V8_CALL_TRACE();
  Isolate* isolate = context->GetIsolate();
  RETURN_HANDLE(Integer, isolate, reinterpret_cast<const JerryValue*>(this)->ToInteger(isolate));
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
  V8_CALL_TRACE();
  const JerryValue* jthis = reinterpret_cast<const JerryValue*>(this);

  jerry_value_t tonumber = jerry_value_to_number (jthis->value());

  double number = 0.0;

  if (jerry_value_is_error (tonumber))
  {
    jerry_release_value (tonumber);
  }
  else
  {
    number = jerry_get_number_value (tonumber);
    jerry_release_value (tonumber);
  }

  return Just(number);
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
  V8_CALL_TRACE();
  jerry_value_t number = jerry_value_to_number(reinterpret_cast<const JerryValue*>(this)->value());

  if (V8_UNLIKELY(jerry_value_is_error(number))) {
      jerry_release_value(number);
      return Nothing<int64_t>();
  }

  double value = jerry_value_as_integer(number);
  jerry_release_value(number);

  return Just(static_cast<int64_t>(value));
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
  V8_CALL_TRACE();
  jerry_value_t number = jerry_value_to_number(reinterpret_cast<const JerryValue*>(this)->value());

  if (V8_UNLIKELY(jerry_value_is_error(number))) {
      jerry_release_value(number);
      return Nothing<int32_t>();
  }

  double value = jerry_value_as_integer(number);
  jerry_release_value(number);

  return Just(static_cast<int32_t>(value));
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
  V8_CALL_TRACE();
  jerry_value_t number = jerry_value_to_number(reinterpret_cast<const JerryValue*>(this)->value());

  if (V8_UNLIKELY(jerry_value_is_error(number))) {
      jerry_release_value(number);
      return Nothing<uint32_t>();
  }

  double value = jerry_value_as_integer(number);
  jerry_release_value(number);

  return Just(static_cast<uint32_t>(value));
}

bool Value::StrictEquals(Local<Value> that) const {
  V8_CALL_TRACE();
  const JerryValue* lhs = reinterpret_cast<const JerryValue*> (this);
  JerryValue* rhs = reinterpret_cast<JerryValue*> (*that);

  jerry_value_t result = jerry_binary_operation (JERRY_BIN_OP_STRICT_EQUAL, lhs->value(), rhs->value());
  bool isEqual = !jerry_value_is_error(result) && jerry_value_is_true(result);
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
  jerry_value_t jobject = reinterpret_cast<JerryValue*>(this)->value();
  jerry_value_t jkey = reinterpret_cast<JerryValue*>(*key)->value();
  jerry_value_t jvalue = reinterpret_cast<JerryValue*>(*value)->value();

  jerry_value_t result = jerry_set_property(jobject, jkey, jvalue);

  if (!jerry_value_is_error(result)) {
      jerry_release_value(result);
      return Just(true);
  }

  JerryIsolate::fromV8(context->GetIsolate())->SetError(result);
  return Nothing<bool>();
}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context, uint32_t index,
                            v8::Local<Value> value) {
  V8_CALL_TRACE();
  jerry_value_t jobject = reinterpret_cast<JerryValue*>(this)->value();
  jerry_value_t jvalue = reinterpret_cast<JerryValue*>(*value)->value();

  jerry_value_t result = jerry_set_property_by_index(jobject, index, jvalue);

  if (!jerry_value_is_error(result)) {
      jerry_release_value(result);
      return Just(true);
  }

  JerryIsolate::fromV8(context->GetIsolate())->SetError(result);
  return Nothing<bool>();
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
  return private_->get;
}

v8::Local<Value> v8::PropertyDescriptor::set() const {
  return private_->set;
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

  jerry_property_descriptor_t prop_desc;
  prop_desc.flags = (JERRY_PROP_IS_VALUE_DEFINED
                     | JERRY_PROP_IS_CONFIGURABLE_DEFINED
                     | JERRY_PROP_IS_ENUMERABLE_DEFINED
                     | JERRY_PROP_IS_WRITABLE_DEFINED);
  if (!(attributes & PropertyAttribute::DontDelete)) {
      prop_desc.flags |= JERRY_PROP_IS_CONFIGURABLE;
  }
  if (!(attributes & PropertyAttribute::DontEnum)) {
      prop_desc.flags |= JERRY_PROP_IS_ENUMERABLE;
  }
  if (!(attributes & PropertyAttribute::ReadOnly)) {
      prop_desc.flags |= JERRY_PROP_IS_WRITABLE;
  }
  prop_desc.value = prop_value->value();
  prop_desc.getter = jerry_create_undefined();
  prop_desc.setter = jerry_create_undefined();

  jerry_value_t result = jerry_define_own_property (obj->value(), prop_name->value(), &prop_desc);
  bool isOk = jerry_value_is_true(result);
  jerry_release_value(result);

  return Just(isOk);
}

Maybe<bool> v8::Object::DefineProperty(v8::Local<v8::Context> context,
                                       v8::Local<Name> key,
                                       PropertyDescriptor& descriptor) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);
  JerryValue* jname = reinterpret_cast<JerryValue*>(*key);

  jerry_property_descriptor_t prop_desc = jerry_property_descriptor_create();

  if (descriptor.has_value()) {
      prop_desc.flags |= JERRY_PROP_IS_VALUE_DEFINED;
      prop_desc.value = jerry_acquire_value(reinterpret_cast<JerryValue*>(*descriptor.value())->value());
  }
  if (descriptor.has_get()) {
      prop_desc.flags |= JERRY_PROP_IS_GET_DEFINED;
      prop_desc.getter = jerry_acquire_value(reinterpret_cast<JerryValue*>(*descriptor.get())->value());
  }
  if (descriptor.has_set()) {
      prop_desc.flags |= JERRY_PROP_IS_SET_DEFINED;
      prop_desc.setter = jerry_acquire_value(reinterpret_cast<JerryValue*>(*descriptor.set())->value());
  }
  if (descriptor.has_configurable()) {
      prop_desc.flags |= JERRY_PROP_IS_CONFIGURABLE_DEFINED;
      if (descriptor.configurable()) {
          prop_desc.flags |= JERRY_PROP_IS_CONFIGURABLE;
      }
  }
  if (descriptor.has_enumerable()) {
      prop_desc.flags |= JERRY_PROP_IS_ENUMERABLE_DEFINED;
      if (descriptor.enumerable()) {
          prop_desc.flags |= JERRY_PROP_IS_ENUMERABLE;
      }
  }
  if (descriptor.has_writable()) {
      prop_desc.flags |= JERRY_PROP_IS_WRITABLE_DEFINED;
      if (descriptor.writable()) {
          prop_desc.flags |= JERRY_PROP_IS_WRITABLE;
      }
  }

  jerry_value_t result = jerry_define_own_property(jobject->value(), jname->value(), &prop_desc);
  jerry_property_descriptor_free(&prop_desc);

  if (!jerry_value_is_error(result))
  {
      jerry_release_value(result);
      return Just(true);
  }

  JerryIsolate::fromV8(context->GetIsolate())->SetError(result);
  return Nothing<bool>();
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
  Isolate* isolate = context->GetIsolate();
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  RETURN_HANDLE(Value, isolate, reinterpret_cast<JerryValue*>(this)->GetProperty(isolate, jkey));
}

MaybeLocal<Value> v8::Object::Get(Local<Context> context, uint32_t index) {
  V8_CALL_TRACE();
  Isolate* isolate = context->GetIsolate();

  RETURN_HANDLE(Value, isolate, reinterpret_cast<JerryValue*>(this)->GetPropertyIdx(isolate, index));
}

MaybeLocal<Value> v8::Object::GetPrivate(Local<Context> context,
                                         Local<Private> key) {
  V8_CALL_TRACE();
  Isolate* isolate = context->GetIsolate();
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  RETURN_HANDLE(Value, isolate, reinterpret_cast<JerryValue*>(this)->GetPrivateProperty(isolate, jkey));
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
  bool isOk = !jerry_value_is_error(result) && jerry_value_is_true(result);
  jerry_release_value(result);

  return Just(isOk);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(Local<Context> context) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);

  int filter = JERRY_PROPERTY_FILTER_TRAVERSE_PROTOTYPE_CHAIN | JERRY_PROPERTY_FILTER_EXLCUDE_NON_ENUMERABLE;
  JerryValue* names = new JerryValue(jerry_object_get_property_names (jobject->value(), static_cast<jerry_property_filter_t>(filter)));

  RETURN_HANDLE(Array, context->GetIsolate(), names);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(
    Local<Context> context, KeyCollectionMode mode,
    PropertyFilter property_filter, IndexFilter index_filter,
    KeyConversionMode key_conversion) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);

  int filter = JERRY_PROPERTY_FILTER_ALL;

  if (mode == v8::KeyCollectionMode::kIncludePrototypes) {
    filter |= JERRY_PROPERTY_FILTER_TRAVERSE_PROTOTYPE_CHAIN;
  }

  if (property_filter & v8::ONLY_WRITABLE) {
    filter |= JERRY_PROPERTY_FILTER_EXLCUDE_NON_WRITABLE;
  }
  if (property_filter & v8::ONLY_ENUMERABLE) {
    filter |= JERRY_PROPERTY_FILTER_EXLCUDE_NON_ENUMERABLE;
  }
  if (property_filter & v8::ONLY_CONFIGURABLE) {
    filter |= JERRY_PROPERTY_FILTER_EXLCUDE_NON_CONFIGURABLE;
  }
  if (property_filter & v8::SKIP_STRINGS) {
    filter |= JERRY_PROPERTY_FILTER_EXLCUDE_STRINGS;
  }
  if (property_filter & v8::SKIP_SYMBOLS) {
    filter |= JERRY_PROPERTY_FILTER_EXLCUDE_SYMBOLS;
  }

  if (index_filter == v8::IndexFilter::kSkipIndices) {
    filter |= JERRY_PROPERTY_FILTER_EXLCUDE_INTEGER_INDICES;
  }

  if (key_conversion == v8::KeyConversionMode::kKeepNumbers) {
    filter |= JERRY_PROPERTY_FILTER_INTEGER_INDICES_AS_NUMBER;
  }

  JerryValue* names = new JerryValue(jerry_object_get_property_names (jobject->value(), static_cast<jerry_property_filter_t>(filter)));
  RETURN_HANDLE(Array, context->GetIsolate(), names);
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);

  JerryValue* names = jobject->GetOwnPropertyNames();
  RETURN_HANDLE(Array, context->GetIsolate(), names);
}

Local<String> v8::Object::GetConstructorName() {
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);

  JerryV8FunctionHandlerData* data = JerryGetFunctionHandlerData(jobject->value());
  Isolate* isolate = JerryIsolate::toV8(JerryIsolate::GetCurrent());
  JerryValue* name;

  if (data == NULL) {
    name = new JerryValue(jerry_create_string((const jerry_char_t*)""));
  }
  else
  {
    JerryFunctionTemplate* function_template = data->function_template;
    JerryValue prop_name(jerry_create_string((const jerry_char_t*)"name"));

    name = function_template->GetFunction()->GetProperty(isolate, &prop_name);
  }

  RETURN_HANDLE(String, isolate, name);
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
  return Just(jerry_value_is_true(result));
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
  bool has_prop = jerry_value_is_true (has_prop_js);
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
  bool has_prop = jerry_value_is_true (has_prop_js);
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
  bool property_exists = jerry_value_is_true(has_prop);
  jerry_release_value(has_prop);

  return Just(property_exists);
}

MaybeLocal<Value> v8::Object::GetRealNamedProperty(Local<Context> context,
                                                   Local<Name> key) {
  V8_CALL_TRACE();
  jerry_value_t jobject = reinterpret_cast<JerryValue*>(this)->value();
  jerry_value_t jkey = reinterpret_cast<JerryValue*>(*key)->value();
  jerry_value_t jcurrent;

  if (jerry_value_is_proxy(jobject)) {
      jcurrent = jerry_get_proxy_target(jobject);
  } else {
      jcurrent = jerry_acquire_value(jobject);
  }

  jerry_value_t property;
  bool found;

  while (true) {
      property = jerry_get_own_property(jcurrent, jkey, jobject, &found);

      if (jerry_value_is_error(property)) {
          jerry_release_value(jcurrent);
          jerry_release_value(property);
          return MaybeLocal<Value>();
      }

      if (found) {
          jerry_release_value(jcurrent);
          RETURN_HANDLE(Value, context->GetIsolate(), new JerryValue(property));
      }

      jerry_value_t jprototype = jerry_get_prototype(jcurrent);
      jerry_release_value(jcurrent);

      if (jerry_value_is_error(jprototype)) {
          jerry_release_value(jprototype);
          return MaybeLocal<Value>();
      }

      jcurrent = jprototype;
  }
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(Local<Context> context,
                                                                    Local<Name> key) {
  V8_CALL_TRACE();
  JerryValue* jobject = reinterpret_cast<JerryValue*>(this);
  jerry_value_t jkey = reinterpret_cast<JerryValue*>(*key)->value();

  jerry_value_t target_object;

  if (jerry_value_is_proxy(jobject->value())) {
      target_object = jerry_get_proxy_target(jobject->value());
  } else {
      target_object = jerry_acquire_value(jobject->value());
  }

  int max_recursion = 4096;

  do {
      jerry_property_descriptor_t prop_desc;

      jerry_value_t result = jerry_get_own_property_descriptor(target_object, jkey, &prop_desc);
      bool found = jerry_value_is_true(result);
      jerry_release_value(result);

      if (found) {
          int attributes = PropertyAttribute::None;

          if (!(prop_desc.flags & JERRY_PROP_IS_CONFIGURABLE)) {
              attributes |= PropertyAttribute::DontDelete;
          }

          if (!(prop_desc.flags & JERRY_PROP_IS_ENUMERABLE)) {
              attributes |= PropertyAttribute::DontEnum;
          }

          uint16_t mask = JERRY_PROP_IS_WRITABLE_DEFINED | JERRY_PROP_IS_WRITABLE;
          if ((prop_desc.flags & mask) == JERRY_PROP_IS_WRITABLE_DEFINED) {
              attributes |= PropertyAttribute::ReadOnly;
          }

          jerry_property_descriptor_free(&prop_desc);
          jerry_release_value(target_object);
          return Just((PropertyAttribute)attributes);
      }

      jerry_value_t new_target_object = jerry_get_prototype(target_object);
      jerry_release_value(target_object);

      target_object = new_target_object;
  } while (!jerry_value_is_null(target_object) && --max_recursion != 0);

  jerry_release_value(target_object);

  return Nothing<PropertyAttribute>();
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

    jerry_value_t context = jobj->GetObjectCreationContext();

    // TODO: remove the hack:
    if (jerry_value_is_undefined(context)) {
        RETURN_HANDLE(Context, GetIsolate(), JerryIsolate::GetCurrent()->CurrentContext()->Copy());
    }

    // Copy the context
    RETURN_HANDLE(Context, GetIsolate(), new JerryValue(context));
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

  JerryIsolate* isolate = JerryIsolate::GetCurrent();

  bool has_error = isolate->HasError();
  jerry_value_t error;

  if (has_error) {
      error = isolate->TakeError();
  }

  isolate->IncTryDepth();
  jerry_value_t result = jerry_call_function(jfunc->value(), jthis->value(), &arguments[0], argc);
  isolate->DecTryDepth();

  if (V8_UNLIKELY(jerry_value_is_error(result))) {
      if (has_error) {
          jerry_release_value(error);
      }

      isolate->SetError(result);
      isolate->ProcessError(false);
      return MaybeLocal<v8::Value>();
  } else if (has_error) {
      if (isolate->HasError()) {
          jerry_release_value(error);
      } else {
          isolate->RestoreError(error);
      }
  }

  RETURN_HANDLE(Value, JerryIsolate::toV8(isolate), new JerryValue(result));
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
  return Local<Value>();
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

  jerry_length_t length = jerry_get_utf8_string_length(jvalue->value());
  jerry_size_t bytes = jerry_substring_to_utf8_char_buffer(jvalue->value(), 0, length, (jerry_char_t *)buffer, capacity);

  if (!(options & String::NO_NULL_TERMINATION) && bytes < capacity) {
      buffer[bytes] = '\0';
  }

  if (options & String::REPLACE_INVALID_UTF8) {
      uint8_t* ptr = (uint8_t*)buffer;
      uint8_t* buffer_end = ptr + bytes;
      ptr += 2;

      while (ptr < buffer_end) {
          if (ptr[-2] >= 0xed && ptr[-2] < 0xee && ptr[-1] >= 0xa0) {
              ptr[-2] = 0xef;
              ptr[-1] = 0xbf;
              ptr[0] = 0xbd;
              ptr += 2;
          }
          ptr++;
      }
  }

  return (int)bytes;
}

int String::WriteOneByte(Isolate* isolate, uint8_t* buffer, int start,
                         int length, int options) const {
  V8_CALL_TRACE();
  const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

  jerry_size_t str_length = jerry_get_string_length(jvalue->value());

  if (length < 0) {
      length = str_length;
  }

  if (start >= str_length || length <= 0) {
      return 0;
  }

  if ((jerry_size_t)(start + length) >= str_length) {
      length = str_length - start;
      options &= ~String::NO_NULL_TERMINATION;
  }

  jerry_char_t* str_data = (jerry_char_t*)malloc(length * 3);
  jerry_size_t bytes = jerry_substring_to_char_buffer(jvalue->value(), start, start + length, str_data, length * 3);
  jerry_char_t* str_ptr = str_data;
  jerry_char_t* str_end = str_data + bytes;

  /* Store the lower 8 bit only. */
  while (str_ptr < str_end) {
      if (*str_ptr < 0xc0) {
          *buffer++ = *str_ptr++;
      } else if (*str_ptr < 0xe0) {
          *buffer++ = (uint8_t) ((str_ptr[0] << 6) | (str_ptr[1] & 0x3f));
          str_ptr += 2;
      } else {
          *buffer++ = (uint8_t) ((str_ptr[1] << 6) | (str_ptr[2] & 0x3f));
          str_ptr += 3;
      }
  }

  free(str_data);

  if (options & String::NO_NULL_TERMINATION) {
      buffer[length] = '\0';
  }

  return (int)length;
}

int String::Write(Isolate* isolate, uint16_t* buffer, int start, int length,
                  int options) const {
  V8_CALL_TRACE();
  const JerryValue* jvalue = reinterpret_cast<const JerryValue*>(this);

  jerry_size_t str_length = jerry_get_string_length(jvalue->value());

  if (length < 0) {
      length = str_length;
  }

  if (start >= str_length || length <= 0) {
      return 0;
  }

  if ((jerry_size_t)(start + length) >= str_length) {
      length = str_length - start;
      options &= ~String::NO_NULL_TERMINATION;
  }

  jerry_char_t* str_data = (jerry_char_t*)malloc(length * 3);
  jerry_size_t bytes = jerry_substring_to_char_buffer(jvalue->value(), start, start + length, str_data, length * 3);
  jerry_char_t* str_ptr = str_data;
  jerry_char_t* str_end = str_data + bytes;

  while (str_ptr < str_end) {
      if (*str_ptr < 0xc0) {
          *buffer++ = *str_ptr++;
      } else if (*str_ptr < 0xe0) {
          *buffer++ = (uint16_t) (((str_ptr[0] & 0x1f) << 6) | (str_ptr[1] & 0x3f));
          str_ptr += 2;
      } else {
          *buffer++ = (uint16_t) (((str_ptr[0] & 0x0f) << 12) | ((str_ptr[1] & 0x3f) << 6) | (str_ptr[2] & 0x3f));
          str_ptr += 3;
      }
  }

  free(str_data);

  if (options & String::NO_NULL_TERMINATION) {
      buffer[length] = '\0';
  }

  return (int)length;
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
  jerry_value_t symbol = reinterpret_cast<const JerryValue*>(this)->value();

  RETURN_HANDLE(Value, Isolate::GetCurrent(), new JerryValue(jerry_get_symbol_description(symbol)));
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
  RETURN_HANDLE(Value, Isolate::GetCurrent(), jobj->GetInternalJerryValue(index));
}

void v8::Object::SetInternalField(int index, v8::Local<Value> value) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  jobj->SetInternalField(index, reinterpret_cast<JerryValue*>(*value));
}

void* v8::Object::SlowGetAlignedPointerFromInternalField(int index) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
  return jobj->GetInternalPointer(index);
}

void v8::Object::SetAlignedPointerInInternalField(int index, void* value) {
  V8_CALL_TRACE();
  JerryValue* jobj = reinterpret_cast<JerryValue*>(this);
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
}

HeapSpaceStatistics::HeapSpaceStatistics()
    : space_name_(nullptr),
      space_size_(0),
      space_used_size_(0),
      space_available_size_(0),
      physical_space_size_(0) {
  V8_CALL_TRACE();
}

HeapCodeStatistics::HeapCodeStatistics()
    : code_and_metadata_size_(0),
      bytecode_and_metadata_size_(0),
      external_script_source_size_(0) {
  V8_CALL_TRACE();
}

const char* V8::GetVersion() {
  V8_CALL_TRACE();
  return "2.4.0-node.0";
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

  v8::Local<ObjectTemplate> templ;
  JerryObjectTemplate* jerry_templ = NULL;

  if (global_template.ToLocal(&templ)) {
      JerryObjectTemplate *object_template = reinterpret_cast<JerryObjectTemplate*>(*templ);

      jerry_value_t old_realm = jerry_set_realm(__handle->value());

      object_template->InstallProperties(__handle->value());

      if (object_template->HasProxyHandler()) {
          JerryValue *interceptor = object_template->Proxify(__handle->clone());
          jerry_realm_set_this (__handle->value(), interceptor->value());
          delete interceptor;
      }

      jerry_set_realm(old_realm);
  }

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
  jerry_value_t name = jerry_create_string_from_utf8((const jerry_char_t*)kContextSecurityTokenKey.c_str());
  jerry_value_t prop = jerry_get_internal_property(ctx->value(), name);
  jerry_release_value(name);

  RETURN_HANDLE(Value, GetIsolate(), new JerryValue(prop));
}

v8::Isolate* Context::GetIsolate() {
  V8_CALL_TRACE();
  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  assert(ctx->IsContextObject());
  return JerryIsolate::toV8(ctx->ContextGetIsolate());
}

v8::Local<v8::Object> Context::Global() {
  V8_CALL_TRACE();

  JerryValue* ctx = reinterpret_cast<JerryValue*>(this);
  RETURN_HANDLE(Object, GetIsolate(), new JerryValue(jerry_realm_get_this(ctx->value())));
}

static jerry_value_t trace_function(const jerry_call_info_t *call_info_p,
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
  V8_CALL_TRACE();
}

MaybeLocal<v8::Object> ObjectTemplate::NewInstance(Local<Context> context) {
  V8_CALL_TRACE();
  JerryObjectTemplate* object_template = reinterpret_cast<JerryObjectTemplate*>(this);

  // TODO: the function template's method should be set as the object's constructor
  JerryValue* new_instance = JerryValue::NewObject();
  object_template->InstallProperties(new_instance->value());

  JerryFunctionTemplate* constructor = object_template->Constructor();

  if (constructor != NULL)
  {
    jerry_set_prototype(new_instance->value(), constructor->GetPrototype());
    constructor->SetFunctionHandlerData(new_instance->value());
  }

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

  if (data == NULL) {
      return false;
  }

  JerryFunctionTemplate* function_template = data->function_template;

  do {
      if (function_template == reinterpret_cast<JerryFunctionTemplate*>(this)) {
          return true;
      }
      function_template = function_template->protoTemplate();
  } while (function_template != NULL);

  return false;
}

Local<External> v8::External::New(Isolate* isolate, void* value) {
  V8_CALL_TRACE();
  RETURN_HANDLE(External, isolate, JerryValue::NewExternal(value));
}

void* External::Value() const {
  V8_CALL_TRACE();
  return reinterpret_cast<const JerryValue*>(this)->GetExternalData();
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

static jerry_value_t JerryNewFromOneByte(const uint8_t* data, int length)
{
  jerry_char_t* str_data = (jerry_char_t*)malloc(length * 2);
  jerry_char_t* str_ptr = str_data;
  const uint8_t* data_end = data + length;

  while (data < data_end) {
      if (*data < 0x80) {
          *str_ptr++ = (jerry_char_t)*data;
      } else {
          str_ptr[0] = 0xc0 | (jerry_char_t)(*data >> 6);
          str_ptr[1] = 0x80 | (jerry_char_t)(*data & 0x3f);
          str_ptr+= 2;
      }
      data++;
  }

  jerry_value_t str = jerry_create_string_sz(str_data, (jerry_size_t)(str_ptr - str_data));
  free(str_data);
  return str;
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate, const uint8_t* data,
                                          NewStringType type, int length) {
  V8_CALL_TRACE();

  if (length == -1) {
      length = (int)strlen((const char*)data);
  }

  if (length >= String::kMaxLength) {
      return Local<String>();
  }

  const uint8_t* data_ptr = data;
  const uint8_t* data_end = data + length;

  while (data_ptr < data_end) {
      if (*data_ptr >= 0x80) {
          break;
      }
      data_ptr++;
  }

  jerry_value_t str;

  if (data_ptr == data_end) {
      /* ASCII characters */
      str = jerry_create_string_sz(data, (jerry_size_t)length);
  } else {
      str = JerryNewFromOneByte(data, length);
  }

  RETURN_HANDLE(String, isolate, new JerryString(str));
}

static jerry_value_t JerryNewFromTwoByte(const uint16_t* data, int length)
{
  jerry_char_t* str_data = (jerry_char_t*)malloc(length * 3);
  jerry_char_t* str_ptr = str_data;
  const uint16_t* data_end = data + length;

  while (data < data_end) {
      if (*data < 0x80) {
          *str_ptr++ = (jerry_char_t)*data;
      } else if (*data < 0x800) {
          str_ptr[0] = 0xc0 | (jerry_char_t)(*data >> 6);
          str_ptr[1] = 0x80 | (jerry_char_t)(*data & 0x3f);
          str_ptr+= 2;
      } else {
          str_ptr[0] = 0xe0 | (jerry_char_t)(*data >> 12);
          str_ptr[1] = 0x80 | (jerry_char_t)((*data >> 6) & 0x3f);
          str_ptr[2] = 0x80 | (jerry_char_t)(*data & 0x3f);
          str_ptr+= 3;
      }
      data++;
  }

  jerry_value_t str = jerry_create_string_sz(str_data, (jerry_size_t)(str_ptr - str_data));

  free(str_data);
  return str;
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
                                          const uint16_t* data,
                                          NewStringType type, int length) {
  V8_CALL_TRACE();

  if (length == -1) {
      const uint16_t* data_ptr = data;

      length = 0;
      while (*data_ptr++) {
          length++;
      }
  }

  if (length >= String::kMaxLength) {
      return Local<String>();
  }

  jerry_value_t str = JerryNewFromTwoByte(data, length);

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

  if (resource->length() >= String::kMaxLength) {
      return Local<String>();
  }

  jerry_value_t str = JerryNewFromTwoByte(resource->data(), (int)resource->length());

  RETURN_HANDLE(String, isolate, new JerryExternalString(str, reinterpret_cast<ExternalStringResourceBase*>(resource), JerryStringType::TWO_BYTE));
}

MaybeLocal<String> v8::String::NewExternalOneByte(
    Isolate* isolate, v8::String::ExternalOneByteStringResource* resource) {
  V8_CALL_TRACE();

  if (resource->length() >= String::kMaxLength) {
      return Local<String>();
  }


  const uint8_t* data = (const uint8_t*)resource->data();
  const uint8_t* data_ptr = data;
  const uint8_t* data_end = data + resource->length();

  while (data_ptr < data_end) {
      if (*data_ptr >= 0x80) {
          break;
      }
      data_ptr++;
  }

  jerry_value_t str;

  if (data_ptr == data_end) {
      /* ASCII characters */
      str = jerry_create_string_sz((const jerry_char_t*)resource->data(), (int)resource->length());
  } else {
      str = JerryNewFromOneByte((const jerry_char_t*)resource->data(), (int)resource->length());
  }

  RETURN_HANDLE(String, isolate, new JerryExternalString(str, reinterpret_cast<ExternalStringResourceBase*>(resource), JerryStringType::ONE_BYTE));
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

  jerry_value_t array_value = jerry_create_array(0);

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

  return jmap->AsLocal<Map>();
}

Local<v8::Set> v8::Set::New(Isolate* isolate) {
  jerry_value_t set = jerry_create_container (JERRY_CONTAINER_TYPE_SET, NULL, 0);
  RETURN_HANDLE(Set, isolate, new JerryValue(set));
}

MaybeLocal<Set> Set::Add(Local<Context> context, Local<Value> key) {
  V8_CALL_TRACE();
  JerryValue* jset = reinterpret_cast<JerryValue*>(this);
  JerryValue* jkey = reinterpret_cast<JerryValue*>(*key);

  jerry_value_t args[] = { jset->value(), jkey->value() };
  jerry_value_t result = JerryIsolate::fromV8(context->GetIsolate())->HelperSetAdd().Call(jerry_create_undefined(), args, 2);
  jerry_release_value(result);

  return jset->AsLocal<Set>();
}

MaybeLocal<Promise::Resolver> Promise::Resolver::New(Local<Context> context) {
  V8_CALL_TRACE();
  JerryValue* jpromise = JerryValue::NewPromise();

  RETURN_HANDLE(Promise::Resolver, context->GetIsolate(), jpromise);
}

Local<Promise> Promise::Resolver::GetPromise() {
  V8_CALL_TRACE();
  JerryValue* jpromise = reinterpret_cast<JerryValue*>(this)->clone();

  // TODO: maybe wrap the promise object into a resolver?
  RETURN_HANDLE(Promise, JerryIsolate::toV8(JerryIsolate::GetCurrent()), jpromise);
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
  jerry_value_t jvalue = reinterpret_cast<JerryValue*>(this)->value();

  v8::Isolate* isolate = JerryIsolate::toV8(JerryIsolate::GetCurrent());
  RETURN_HANDLE(Value, isolate, JerryValue::TryCreateValue(isolate, jerry_get_promise_result(jvalue)));
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
  jerry_value_t proxy = reinterpret_cast<JerryValue*>(this)->value();
  RETURN_HANDLE(Value, GetIsolate(), new JerryValue(jerry_get_proxy_target(proxy)));
}

Local<Value> Proxy::GetHandler() {
  jerry_value_t proxy = reinterpret_cast<JerryValue*>(this)->value();
  RETURN_HANDLE(Value, GetIsolate(), new JerryValue(jerry_get_proxy_handler(proxy)));
}

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
  V8_CALL_TRACE();
  const JerryValue* arrayBuffer = reinterpret_cast<const JerryValue*>(this);
  return jerry_value_is_true(jerry_is_arraybuffer_detachable(arrayBuffer->value()));
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
  V8_CALL_TRACE();
  const JerryValue* jarray = reinterpret_cast<const JerryValue*> (this);
  jerry_value_t buffer;
  jerry_length_t start;

  if (jarray->IsTypedArray()) {
      buffer = jerry_get_typedarray_buffer (jarray->value(), &start, NULL);
  } else if (jarray->IsDataView()) {
      buffer = jerry_get_dataview_buffer (jarray->value(), &start, NULL);
  } else {
      abort();
  }

  jerry_length_t length = jerry_arraybuffer_read (buffer, start, (uint8_t*)dest, byte_length);
  jerry_release_value(buffer);
  return length;
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
  V8_CALL_TRACE();
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
  JerryIsolate::fromV8(this)->SetAbortOnUncaughtExceptionCallback(callback);
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
  JerryIsolate::fromV8(this)->SetPrepareStackTraceCallback(callback);
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
}

size_t Isolate::NumberOfHeapSpaces() {
  V8_CALL_TRACE();
  return 0;
}

bool Isolate::GetHeapSpaceStatistics(HeapSpaceStatistics* space_statistics,
                                     size_t index) {
  V8_CALL_TRACE();
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
  V8_CALL_TRACE();
  JerryValue* jFunc = reinterpret_cast<JerryValue*>(*v8_function);
  JerryIsolate::fromV8(this)->EnqueueMicrotask(jFunc);
}

void Isolate::SetMicrotasksPolicy(MicrotasksPolicy policy) {
  V8_CALL_TRACE();
  // Leave this empty since node uses only MicrotasksPolicy::kExplicit
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
  JerryIsolate::fromV8(v8_isolate)->RunMicrotasks();
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

  jerry_size_t size = jerry_get_utf8_string_size(value);
  length_ = (int)size;

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
    delete[] buffer;

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
  V8_CALL_TRACE();
  JerryValue* jmap = reinterpret_cast<JerryValue*>(this);
  jerry_value_t map_value = jmap->value();

  jerry_value_t array_value;

  if (jerry_value_is_undefined (map_value) || jerry_value_is_null (map_value))
  {
    array_value = jerry_create_undefined();
  }
  else
  {
    array_value = jerry_get_array_from_container(map_value, is_key_value);
  }
  RETURN_HANDLE(Array, Isolate::GetCurrent(), new JerryValue(array_value));
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
