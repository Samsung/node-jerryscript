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
#include "v8jerry_callback.hpp"
#include "v8jerry_handlescope.hpp"
#include "v8jerry_flags.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_platform.hpp"
#include "v8jerry_templates.hpp"
#include "v8jerry_value.hpp"

// /* Remove the comments to enable trace macros */
#define USE_TRACE

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
  UNIMPLEMENTED(1042);
  return NULL;
}

void V8::MoveGlobalReference(internal::Address** from, internal::Address** to) {
  UNIMPLEMENTED(1076);
}

void V8::MakeWeak(i::Address* location, void* parameter,
                  WeakCallbackInfo<void>::Callback weak_callback,
                  WeakCallbackType type) {
  UNIMPLEMENTED(1090);
}

void V8::MakeWeak(i::Address** location_addr) {
  UNIMPLEMENTED(1096);
}

void* V8::ClearWeak(i::Address* location) {
  UNIMPLEMENTED(1100);
  return NULL;
}

void V8::DisposeGlobal(i::Address* location) {
  UNIMPLEMENTED(1108);
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
  UNIMPLEMENTED(1132);
}

void V8::ToLocalEmpty() {
  UNIMPLEMENTED(1136);
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
  UNIMPLEMENTED(1365);
}

void Template::SetAccessorProperty(v8::Local<v8::Name> name,
                                   v8::Local<FunctionTemplate> getter,
                                   v8::Local<FunctionTemplate> setter,
                                   v8::PropertyAttribute attribute,
                                   v8::AccessControl access_control) {
  UNIMPLEMENTED(1390);
}

Local<ObjectTemplate> FunctionTemplate::PrototypeTemplate() {
  UNIMPLEMENTED(1420);
  return Local<ObjectTemplate>();
}

void FunctionTemplate::Inherit(v8::Local<FunctionTemplate> value) {
  UNIMPLEMENTED(1452);
}

Local<FunctionTemplate> FunctionTemplate::New(
    Isolate* isolate, FunctionCallback callback, v8::Local<Value> data,
    v8::Local<Signature> signature, int length, ConstructorBehavior behavior,
    SideEffectType side_effect_type, const CFunction*) {
  UNIMPLEMENTED(1499);
  return Local<FunctionTemplate>();
}

Local<Signature> Signature::New(Isolate* isolate,
                                Local<FunctionTemplate> receiver) {
  UNIMPLEMENTED(1525);
  return Local<Signature>();
}

Local<ObjectTemplate> FunctionTemplate::InstanceTemplate() {
  UNIMPLEMENTED(1604);
  return Local<ObjectTemplate>();
}

void FunctionTemplate::SetClassName(Local<String> name) {
  UNIMPLEMENTED(1632);
}

Local<ObjectTemplate> ObjectTemplate::New(
    Isolate* isolate, v8::Local<FunctionTemplate> constructor) {
  UNIMPLEMENTED(1666);
  return Local<ObjectTemplate>();
}

void ObjectTemplate::SetAccessor(v8::Local<String> name,
                                 AccessorGetterCallback getter,
                                 AccessorSetterCallback setter,
                                 v8::Local<Value> data, AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
  UNIMPLEMENTED(1781);
}

void ObjectTemplate::SetAccessor(v8::Local<Name> name,
                                 AccessorNameGetterCallback getter,
                                 AccessorNameSetterCallback setter,
                                 v8::Local<Value> data, AccessControl settings,
                                 PropertyAttribute attribute,
                                 v8::Local<AccessorSignature> signature,
                                 SideEffectType getter_side_effect_type,
                                 SideEffectType setter_side_effect_type) {
  UNIMPLEMENTED(1794);
}

void ObjectTemplate::SetHandler(
    const NamedPropertyHandlerConfiguration& config) {
  UNIMPLEMENTED(1887);
}

void ObjectTemplate::SetHandler(
    const IndexedPropertyHandlerConfiguration& config) {
  UNIMPLEMENTED(1969);
}

void ObjectTemplate::SetInternalFieldCount(int value) {
  UNIMPLEMENTED(2004);
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
  UNIMPLEMENTED(2044);
}

Local<Script> UnboundScript::BindToCurrentContext() {
  UNIMPLEMENTED(2064);
  return Local<Script>();
}

MaybeLocal<Value> Script::Run(Local<Context> context) {
  UNIMPLEMENTED(2137);
  return MaybeLocal<Value>();
}

Local<PrimitiveArray> ScriptOrModule::GetHostDefinedOptions() {
  UNIMPLEMENTED(2164);
  return Local<PrimitiveArray>();
}

Local<PrimitiveArray> PrimitiveArray::New(Isolate* v8_isolate, int length) {
  UNIMPLEMENTED(2180);
}

int PrimitiveArray::Length() const {
  UNIMPLEMENTED(2189);
  return 0;
}

void PrimitiveArray::Set(Isolate* v8_isolate, int index,
                         Local<Primitive> item) {
  UNIMPLEMENTED(2194);
}

Local<Primitive> PrimitiveArray::Get(Isolate* v8_isolate, int index) {
  UNIMPLEMENTED(2207);
  return Local<Primitive>();
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
  UNIMPLEMENTED(2309);
  return 0;
}

Maybe<bool> Module::InstantiateModule(Local<Context> context,
                                      Module::ResolveCallback callback) {
  UNIMPLEMENTED(2311);
  return Just(false);
}

MaybeLocal<Value> Module::Evaluate(Local<Context> context) {
  UNIMPLEMENTED(2322);
  return MaybeLocal<Value>();
}

Local<Module> Module::CreateSyntheticModule(
    Isolate* isolate, Local<String> module_name,
    const std::vector<Local<v8::String>>& export_names,
    v8::Module::SyntheticModuleEvaluationSteps evaluation_steps) {
  UNIMPLEMENTED(2341);
  return Local<Module>();
}

Maybe<bool> Module::SetSyntheticModuleExport(Isolate* isolate,
                                             Local<String> export_name,
                                             Local<v8::Value> export_value) {
  UNIMPLEMENTED(2359);
  return Just(false);
}

MaybeLocal<UnboundScript> ScriptCompiler::CompileUnboundScript(
    Isolate* v8_isolate, Source* source, CompileOptions options,
    NoCacheReason no_cache_reason) {
  UNIMPLEMENTED(2463);
  return MaybeLocal<UnboundScript>();
}

MaybeLocal<Module> ScriptCompiler::CompileModule(
    Isolate* isolate, Source* source, CompileOptions options,
    NoCacheReason no_cache_reason) {
  UNIMPLEMENTED(2489);
  return MaybeLocal<Module>();
}

MaybeLocal<Function> ScriptCompiler::CompileFunctionInContext(
    Local<Context> v8_context, Source* source, size_t arguments_count,
    Local<String> arguments[], size_t context_extension_count,
    Local<Object> context_extensions[], CompileOptions options,
    NoCacheReason no_cache_reason,
    Local<ScriptOrModule>* script_or_module_out) {
  UNIMPLEMENTED(2531);
  return MaybeLocal<Function>();
}

uint32_t ScriptCompiler::CachedDataVersionTag() {
  UNIMPLEMENTED(2665);
  return 0;
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundScript> unbound_script) {
  UNIMPLEMENTED(2671);
  return NULL;
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCache(
    Local<UnboundModuleScript> unbound_module_script) {
  UNIMPLEMENTED(2681);
  return NULL;
}

ScriptCompiler::CachedData* ScriptCompiler::CreateCodeCacheForFunction(
    Local<Function> function) {
  UNIMPLEMENTED(2690);
}

MaybeLocal<Script> Script::Compile(Local<Context> context, Local<String> source,
                                   ScriptOrigin* origin) {
  UNIMPLEMENTED(2700);
  return MaybeLocal<Script>();
}

v8::TryCatch::TryCatch(v8::Isolate* isolate)
    : isolate_(NULL),
      next_(NULL),
      is_verbose_(false),
      can_continue_(true),
      capture_message_(true),
      rethrow_(false),
      has_terminated_(false) {
  UNIMPLEMENTED(2712);
}

v8::TryCatch::~TryCatch() {
  UNIMPLEMENTED(2727);
}

bool v8::TryCatch::HasCaught() const {
  UNIMPLEMENTED(2761);
  return false;
}

bool v8::TryCatch::CanContinue() const {
  UNIMPLEMENTED(2766);
  return false;
}

bool v8::TryCatch::HasTerminated() const {
  UNIMPLEMENTED(2768);
  return false;
}

v8::Local<v8::Value> v8::TryCatch::ReThrow() {
  UNIMPLEMENTED(2770);
  return v8::Local<Value>();
}

v8::Local<Value> v8::TryCatch::Exception() const {
  UNIMPLEMENTED(2776);
  return v8::Local<Value>();
}

v8::Local<v8::Message> v8::TryCatch::Message() const {
  UNIMPLEMENTED(2809);
  return v8::Local<v8::Message>();
}

void v8::TryCatch::SetVerbose(bool value) {
  UNIMPLEMENTED(2835);
}

bool v8::TryCatch::IsVerbose() const {
  UNIMPLEMENTED(2837);
  return false;
}

Local<String> Message::Get() const {
  UNIMPLEMENTED(2843);
  return Local<String>();
}

v8::Isolate* Message::GetIsolate() const {
  UNIMPLEMENTED(2854);
  return NULL;
}

ScriptOrigin Message::GetScriptOrigin() const {
  UNIMPLEMENTED(2859);
  return ScriptOrigin(Local<v8::Value>());
}

v8::Local<Value> Message::GetScriptResourceName() const {
  UNIMPLEMENTED(2867);
  return v8::Local<Value>();
}

v8::Local<v8::StackTrace> Message::GetStackTrace() const {
  UNIMPLEMENTED(2871);
  return Local<v8::StackTrace>();
}

Maybe<int> Message::GetLineNumber(Local<Context> context) const {
  UNIMPLEMENTED(2882);
  return Just(0);
}

int Message::ErrorLevel() const {
  UNIMPLEMENTED(2909);
  return 0;
}

Maybe<int> Message::GetStartColumn(Local<Context> context) const {
  UNIMPLEMENTED(2943);
  return Just(0);
}

Maybe<int> Message::GetEndColumn(Local<Context> context) const {
  UNIMPLEMENTED(2960);
  return Just(0);
}

MaybeLocal<String> Message::GetSourceLine(Local<Context> context) const {
  UNIMPLEMENTED(2978);
  return MaybeLocal<String>();
}

Local<StackFrame> StackTrace::GetFrame(Isolate* v8_isolate,
                                       uint32_t index) const {
  UNIMPLEMENTED(2995);
  return Local<StackFrame>();
}

int StackTrace::GetFrameCount() const {
  UNIMPLEMENTED(3005);
  return 0;
}

Local<StackTrace> StackTrace::CurrentStackTrace(Isolate* isolate,
                                                int frame_limit,
                                                StackTraceOptions options) {
  UNIMPLEMENTED(3009);
  return Local<StackTrace>();
}

int StackFrame::GetLineNumber() const {
  UNIMPLEMENTED(3021);
  return 0;
}

int StackFrame::GetColumn() const {
  UNIMPLEMENTED(3025);
  return 0;
}

int StackFrame::GetScriptId() const {
  UNIMPLEMENTED(3029);
  return 0;
}

Local<String> StackFrame::GetScriptName() const {
  UNIMPLEMENTED(3033);
  return Local<String>();
}

Local<String> StackFrame::GetFunctionName() const {
  UNIMPLEMENTED(3054);
  return Local<String>();
}

bool StackFrame::IsEval() const {
  UNIMPLEMENTED(3064);
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
  UNIMPLEMENTED(3535);
  return MaybeLocal<String>();
}

MaybeLocal<String> Value::ToDetailString(Local<Context> context) const {
  UNIMPLEMENTED(3546);
  return MaybeLocal<String>();
}

MaybeLocal<Object> Value::ToObject(Local<Context> context) const {
  UNIMPLEMENTED(3556);
  return MaybeLocal<Object>();
}

bool Value::BooleanValue(Isolate* v8_isolate) const {
  UNIMPLEMENTED(3578);
  return false;
}

Local<Boolean> Value::ToBoolean(Isolate* v8_isolate) const {
  UNIMPLEMENTED(3583);
  return Local<Boolean>();
}

MaybeLocal<Number> Value::ToNumber(Local<Context> context) const {
  UNIMPLEMENTED(3589);
  return MaybeLocal<Number>();
}

MaybeLocal<Integer> Value::ToInteger(Local<Context> context) const {
  UNIMPLEMENTED(3600);
  return MaybeLocal<Integer>();
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
  UNIMPLEMENTED(3761);
}

void* v8::BackingStore::Data() const {
  UNIMPLEMENTED(3766);
  return NULL;
}

size_t v8::BackingStore::ByteLength() const {
  UNIMPLEMENTED(3770);
  return 0;
}

std::shared_ptr<v8::BackingStore> v8::ArrayBuffer::GetBackingStore() {
  UNIMPLEMENTED(3794);
  return NULL;
}

std::shared_ptr<v8::BackingStore> v8::SharedArrayBuffer::GetBackingStore() {
  UNIMPLEMENTED(3806);
  return NULL;
}

void v8::ArrayBuffer::CheckCast(Value* that) {
  UNIMPLEMENTED(3817);
}

void v8::ArrayBufferView::CheckCast(Value* that) {
  UNIMPLEMENTED(3824);
}

void v8::TypedArray::CheckCast(Value* that) {
  UNIMPLEMENTED(3832);
}

void v8::DataView::CheckCast(Value* that) {
  UNIMPLEMENTED(3851);
}

void v8::SharedArrayBuffer::CheckCast(Value* that) {
  UNIMPLEMENTED(3857);
}

void v8::Date::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3864);
}

Maybe<double> Value::NumberValue(Local<Context> context) const {
  UNIMPLEMENTED(3905);
  return Just((double) 0);
}

Maybe<int64_t> Value::IntegerValue(Local<Context> context) const {
  UNIMPLEMENTED(3917);
  return Just((int64_t) 0);
}

Maybe<int32_t> Value::Int32Value(Local<Context> context) const {
  UNIMPLEMENTED(3931);
  return Just((int32_t) 0);
}

Maybe<uint32_t> Value::Uint32Value(Local<Context> context) const {
  UNIMPLEMENTED(3944);
  return Just((uint32_t) 0);
}

bool Value::StrictEquals(Local<Value> that) const {
  UNIMPLEMENTED(3989);
  return false;
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

v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value)
    : private_(NULL) {
  UNIMPLEMENTED(4093);
}

v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> value,
                                           bool writable)
    : private_(NULL) {
  UNIMPLEMENTED(4099);
}

v8::PropertyDescriptor::PropertyDescriptor(v8::Local<v8::Value> get,
                                           v8::Local<v8::Value> set)
    : private_(NULL) {
  UNIMPLEMENTED(4107);
}

v8::PropertyDescriptor::~PropertyDescriptor() {
  UNIMPLEMENTED(4116);
}

v8::Local<Value> v8::PropertyDescriptor::value() const {
  UNIMPLEMENTED(4118);
  return Local<Value>();
}

v8::Local<Value> v8::PropertyDescriptor::get() const {
  UNIMPLEMENTED(4123);
  return Local<Value>();
}

v8::Local<Value> v8::PropertyDescriptor::set() const {
  UNIMPLEMENTED(4128);
  return Local<Value>();
}

bool v8::PropertyDescriptor::has_value() const {
  UNIMPLEMENTED(4133);
  return false;
}

bool v8::PropertyDescriptor::has_get() const {
  UNIMPLEMENTED(4136);
  return false;
}

bool v8::PropertyDescriptor::has_set() const {
  UNIMPLEMENTED(4139);
  return false;
}

bool v8::PropertyDescriptor::writable() const {
  UNIMPLEMENTED(4143);
  return false;
}

bool v8::PropertyDescriptor::has_writable() const {
  UNIMPLEMENTED(4148);
  return false;
}

void v8::PropertyDescriptor::set_enumerable(bool enumerable) {
  UNIMPLEMENTED(4152);
}

bool v8::PropertyDescriptor::enumerable() const {
  UNIMPLEMENTED(4156);
  return false;
}

bool v8::PropertyDescriptor::has_enumerable() const {
  UNIMPLEMENTED(4161);
  return false;
}

void v8::PropertyDescriptor::set_configurable(bool configurable) {
  UNIMPLEMENTED(4165);
}

bool v8::PropertyDescriptor::configurable() const {
  UNIMPLEMENTED(4169);
  return false;
}

bool v8::PropertyDescriptor::has_configurable() const {
  UNIMPLEMENTED(4174);
  return false;
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
  UNIMPLEMENTED(4368);
  return MaybeLocal<Array>();
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
  UNIMPLEMENTED(4420);
  return Just(false);
}

Maybe<bool> v8::Object::Delete(Local<Context> context, Local<Value> key) {
  UNIMPLEMENTED(4435);
  return Just(false);
}

Maybe<bool> v8::Object::DeletePrivate(Local<Context> context,
                                      Local<Private> key) {
  UNIMPLEMENTED(4459);
  return Just(false);
}

Maybe<bool> v8::Object::Has(Local<Context> context, Local<Value> key) {
  UNIMPLEMENTED(4475);
  return Just(false);
}

Maybe<bool> v8::Object::HasPrivate(Local<Context> context, Local<Private> key) {
  UNIMPLEMENTED(4497);
  return Just(false);
}

Maybe<bool> v8::Object::Delete(Local<Context> context, uint32_t index) {
  UNIMPLEMENTED(4501);
  return Just(false);
}

Maybe<bool> v8::Object::Has(Local<Context> context, uint32_t index) {
  UNIMPLEMENTED(4511);
  return Just(false);
}

Maybe<bool> Object::SetAccessor(Local<Context> context, Local<Name> name,
                                AccessorNameGetterCallback getter,
                                AccessorNameSetterCallback setter,
                                MaybeLocal<Value> data, AccessControl settings,
                                PropertyAttribute attribute,
                                SideEffectType getter_side_effect_type,
                                SideEffectType setter_side_effect_type) {
  UNIMPLEMENTED(4557);
  return Just(false);
}

Maybe<bool> v8::Object::HasOwnProperty(Local<Context> context,
                                       Local<Name> key) {
  UNIMPLEMENTED(4611);
  return Just(false);
}

MaybeLocal<Value> v8::Object::GetRealNamedProperty(Local<Context> context,
                                                   Local<Name> key) {
  UNIMPLEMENTED(4737);
  return MaybeLocal<Value>();
}

Maybe<PropertyAttribute> v8::Object::GetRealNamedPropertyAttributes(
    Local<Context> context, Local<Name> key) {
  UNIMPLEMENTED(4752);
  return Just(PropertyAttribute());
}

Local<v8::Object> v8::Object::Clone() {
  UNIMPLEMENTED(4772);
  return Local<v8::Object>();
}

Local<v8::Context> v8::Object::CreationContext() {
  UNIMPLEMENTED(4781);
  return Local<v8::Context>();
}

int v8::Object::GetIdentityHash() {
  UNIMPLEMENTED(4787);
  return 0;
}

MaybeLocal<Function> Function::New(Local<Context> context,
                                   FunctionCallback callback, Local<Value> data,
                                   int length, ConstructorBehavior behavior,
                                   SideEffectType side_effect_type) {
  UNIMPLEMENTED(4846);
  return MaybeLocal<Function>();
}

MaybeLocal<Object> Function::NewInstance(Local<Context> context, int argc,
                                         v8::Local<v8::Value> argv[]) const {
  UNIMPLEMENTED(4860);
  return MaybeLocal<Object>();
}

MaybeLocal<v8::Value> Function::Call(Local<Context> context,
                                     v8::Local<v8::Value> recv, int argc,
                                     v8::Local<v8::Value> argv[]) {
  UNIMPLEMENTED(4913);
  return MaybeLocal<v8::Value>();
}

void Function::SetName(v8::Local<v8::String> name) {
  UNIMPLEMENTED(4934);
}

Local<Value> Function::GetDebugName() const {
  UNIMPLEMENTED(4970);
  return Local<Value>();
}

Local<v8::Value> Function::GetBoundFunction() const {
  UNIMPLEMENTED(5058);
  return Local<v8::Value>();
}

int Name::GetIdentityHash() {
  UNIMPLEMENTED(5069);
  return 0;
}

int String::Length() const {
  UNIMPLEMENTED(5074);
  return 0;
}

int String::Utf8Length(Isolate* isolate) const {
  UNIMPLEMENTED(5199);
  return 0;
}

int String::WriteUtf8(Isolate* v8_isolate, char* buffer, int capacity,
                      int* nchars_ref, int options) const {
  UNIMPLEMENTED(5351);
  return 0;
}

int String::WriteOneByte(Isolate* isolate, uint8_t* buffer, int start,
                         int length, int options) const {
  UNIMPLEMENTED(5389);
  return 0;
}

int String::Write(Isolate* isolate, uint16_t* buffer, int start, int length,
                  int options) const {
  UNIMPLEMENTED(5395);
  return 0;
}

bool v8::String::IsExternal() const {
  UNIMPLEMENTED(5401);
  return false;
}

bool v8::String::IsExternalOneByte() const {
  UNIMPLEMENTED(5406);
  return false;
}

void v8::String::VerifyExternalStringResource(
    v8::String::ExternalStringResource* value) const {
  UNIMPLEMENTED(5411);
}

String::ExternalStringResource* String::GetExternalStringResourceSlow() const {
  UNIMPLEMENTED(5458);
  return NULL;
}

const v8::String::ExternalOneByteStringResource*
v8::String::GetExternalOneByteStringResource() const {
  UNIMPLEMENTED(5496);
  return NULL;
}

Local<Value> Symbol::Description() const {
  UNIMPLEMENTED(5511);
  return Local<Value>();
}

double Number::Value() const {
  UNIMPLEMENTED(5535);
  return 0;
}

bool Boolean::Value() const {
  UNIMPLEMENTED(5540);
  return false;
}

int64_t Integer::Value() const {
  UNIMPLEMENTED(5545);
  return 0;
}

int32_t Int32::Value() const {
  UNIMPLEMENTED(5554);
  return 0;
}

uint32_t Uint32::Value() const {
  UNIMPLEMENTED(5563);
  return 0;
}

int v8::Object::InternalFieldCount() {
  UNIMPLEMENTED(5572);
  return 0;
}

Local<Value> v8::Object::SlowGetInternalField(int index) {
  UNIMPLEMENTED(5586);
  return Local<Value>();
}

void v8::Object::SetInternalField(int index, v8::Local<Value> value) {
  UNIMPLEMENTED(5595);
}

void* v8::Object::SlowGetAlignedPointerFromInternalField(int index) {
  UNIMPLEMENTED(5603);
  return NULL;
}

void v8::Object::SetAlignedPointerInInternalField(int index, void* value) {
  UNIMPLEMENTED(5614);
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
  jerry_value_t global = jerry_get_global_object();
  jerry_release_value (global);
  RETURN_HANDLE(Object, GetIsolate(), new JerryValue(global));
}

Local<v8::Object> Context::GetExtrasBindingObject() {
  UNIMPLEMENTED(6075);
  return Local<v8::Object>();
}

void Context::AllowCodeGenerationFromStrings(bool allow) {
  UNIMPLEMENTED(6082);
}

MaybeLocal<v8::Object> ObjectTemplate::NewInstance(Local<Context> context) {
  UNIMPLEMENTED(6143);
  return MaybeLocal<v8::Object>();
}

MaybeLocal<v8::Function> FunctionTemplate::GetFunction(Local<Context> context) {
  UNIMPLEMENTED(6177);
  return MaybeLocal<v8::Function>();
}

bool FunctionTemplate::HasInstance(v8::Local<v8::Value> value) {
  UNIMPLEMENTED(6214);
  return false;
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

  jerry_value_t str_value = jerry_create_string_sz_from_utf8((const jerry_char_t*)data, length);

  RETURN_HANDLE(String, isolate, new JerryValue(str_value));
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate, const uint8_t* data,
                                          NewStringType type, int length) {
  V8_CALL_TRACE();
  return String::NewFromUtf8(isolate, (const char*)data, type, length);
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
                                          const uint16_t* data,
                                          NewStringType type, int length) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  std::string dest = convert.to_bytes(reinterpret_cast<const char16_t*>(data),
                                      reinterpret_cast<const char16_t*>(data + length));

  return String::NewFromUtf8(isolate, dest.c_str(), type, dest.size());
}

Local<String> v8::String::Concat(Isolate* v8_isolate, Local<String> left,
                                 Local<String> right) {
  UNIMPLEMENTED(6339);
  return Local<String>();
}

MaybeLocal<String> v8::String::NewExternalTwoByte(
    Isolate* isolate, v8::String::ExternalStringResource* resource) {
  UNIMPLEMENTED(6357);
  return MaybeLocal<String>();
}

MaybeLocal<String> v8::String::NewExternalOneByte(
    Isolate* isolate, v8::String::ExternalOneByteStringResource* resource) {
  UNIMPLEMENTED(6379);
  return MaybeLocal<String>();
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
  UNIMPLEMENTED(6753);
  return Local<v8::Array>();
}

Local<v8::Array> v8::Array::New(Isolate* isolate, Local<Value>* elements,
                                size_t length) {
  UNIMPLEMENTED(6753);
  return Local<v8::Array>();
}

uint32_t v8::Array::Length() const {
  UNIMPLEMENTED(6783);
  return 0;
}

Local<v8::Map> v8::Map::New(Isolate* isolate) {
  UNIMPLEMENTED(6793);
  return Local<v8::Map>();
}

MaybeLocal<Map> Map::Set(Local<Context> context, Local<Value> key,
                         Local<Value> value) {
  UNIMPLEMENTED(6827);
  return MaybeLocal<Map>();
}

Local<v8::Set> v8::Set::New(Isolate* isolate) {
  UNIMPLEMENTED(6925);
  return Local<Set>();
}

MaybeLocal<Set> Set::Add(Local<Context> context, Local<Value> key) {
  UNIMPLEMENTED(6946);
  return MaybeLocal<Set>();
}

MaybeLocal<Promise::Resolver> Promise::Resolver::New(Local<Context> context) {
  UNIMPLEMENTED(7027);
  return MaybeLocal<Promise::Resolver>();
}

Local<Promise> Promise::Resolver::GetPromise() {
  UNIMPLEMENTED(7036);
  return Local<Promise>();
}

Maybe<bool> Promise::Resolver::Resolve(Local<Context> context,
                                       Local<Value> value) {
  UNIMPLEMENTED(7041);
  return Just(true);
}

Maybe<bool> Promise::Resolver::Reject(Local<Context> context,
                                      Local<Value> value) {
  UNIMPLEMENTED(7059);
  return Just(true);
}

Local<Value> Promise::Result() {
  UNIMPLEMENTED(7133);
  return Local<Value>();
}

Promise::PromiseState Promise::State() {
  UNIMPLEMENTED(7144);
  return kPending;
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

Local<ArrayBuffer> v8::ArrayBuffer::New(Isolate* isolate, size_t byte_length) {
  UNIMPLEMENTED(7439);
  return Local<ArrayBuffer>();
}

Local<ArrayBuffer> v8::ArrayBuffer::New(
    Isolate* isolate, std::shared_ptr<BackingStore> backing_store) {
  UNIMPLEMENTED(7478);
  return Local<ArrayBuffer>();
}

std::unique_ptr<v8::BackingStore> v8::ArrayBuffer::NewBackingStore(
    Isolate* isolate, size_t byte_length) {
  UNIMPLEMENTED(7495);
  return NULL;
}

std::unique_ptr<v8::BackingStore> v8::ArrayBuffer::NewBackingStore(
    void* data, size_t byte_length, BackingStoreDeleterCallback deleter,
    void* deleter_data) {
  UNIMPLEMENTED(7512);
  return NULL;
}

Local<ArrayBuffer> v8::ArrayBufferView::Buffer() {
  UNIMPLEMENTED(7523);
  return Local<ArrayBuffer>();
}

size_t v8::ArrayBufferView::CopyContents(void* dest, size_t byte_length) {
  UNIMPLEMENTED(7539);
  return 0;
}

bool v8::ArrayBufferView::HasBuffer() const {
  UNIMPLEMENTED(7560);
  return false;
}

size_t v8::ArrayBufferView::ByteOffset() {
  UNIMPLEMENTED(7567);
  return 0;
}

size_t v8::ArrayBufferView::ByteLength() {
  UNIMPLEMENTED(7572);
  return 0;
}

void v8::ArrayBuffer::Detach() {
  UNIMPLEMENTED(7424);
}

size_t v8::ArrayBuffer::ByteLength() const {
  UNIMPLEMENTED(7434);
  return 0;
}

size_t v8::TypedArray::Length() {
  UNIMPLEMENTED(7577);
  return 0;
}

Local<DataView> DataView::New(Local<ArrayBuffer> array_buffer,
                              size_t byte_offset, size_t byte_length) {
  UNIMPLEMENTED(7628);
  return Local<DataView>();
}

Local<SharedArrayBuffer> v8::SharedArrayBuffer::New(
    Isolate* isolate, std::shared_ptr<BackingStore> backing_store) {
  UNIMPLEMENTED(7799);
  return Local<SharedArrayBuffer>();
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
  UNIMPLEMENTED(7917);
  return Local<Number>();
}

Local<Integer> v8::Integer::New(Isolate* isolate, int32_t value) {
  UNIMPLEMENTED(7928);
  return Local<Integer>();
}

Local<Integer> v8::Integer::NewFromUnsigned(Isolate* isolate, uint32_t value) {
  UNIMPLEMENTED(7939);
  return Local<Integer>();
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
  UNIMPLEMENTED(8353);
}

Isolate::DisallowJavascriptExecutionScope::~DisallowJavascriptExecutionScope() {
  UNIMPLEMENTED(8377);
}

Isolate::AllowJavascriptExecutionScope::AllowJavascriptExecutionScope(
    Isolate* isolate) {
  UNIMPLEMENTED(8393);
}

Isolate::AllowJavascriptExecutionScope::~AllowJavascriptExecutionScope() {
  UNIMPLEMENTED(8404);
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
  UNIMPLEMENTED(9140);
}

String::Utf8Value::~Utf8Value() {
  UNIMPLEMENTED(9155);
}

String::Value::Value(v8::Isolate* isolate, v8::Local<v8::Value> obj)
    : str_(nullptr), length_(0) {
  UNIMPLEMENTED(9157);
}

String::Value::~Value() {
  UNIMPLEMENTED(9172);
}

v8::Local<Value> Exception::RangeError(v8::Local<v8::String> raw_message) {
  UNIMPLEMENTED(9190);
  return v8::Local<Value>();
}

v8::Local<Value> Exception::TypeError(v8::Local<v8::String> raw_message) {
  UNIMPLEMENTED(9193);
  return v8::Local<Value>();
}

v8::Local<Value> Exception::Error(v8::Local<v8::String> raw_message) {
  UNIMPLEMENTED(9194);
  return v8::Local<Value>();
}

Local<Message> Exception::CreateMessage(Isolate* isolate,
                                        Local<Value> exception) {
  UNIMPLEMENTED(9198);
  return Local<Message>();
}

v8::MaybeLocal<v8::Array> v8::Object::PreviewEntries(bool* is_key_value) {
  UNIMPLEMENTED(9826);
  return v8::MaybeLocal<v8::Array>();
}

void CpuProfiler::UseDetailedSourcePositionsForProfiling(Isolate* isolate) {
  V8_CALL_TRACE();
}

void HeapSnapshot::Delete() {
  UNIMPLEMENTED(10604);
}

void HeapSnapshot::Serialize(OutputStream* stream,
                             HeapSnapshot::SerializationFormat format) const {
  UNIMPLEMENTED(10636);
}

const HeapSnapshot* HeapProfiler::TakeHeapSnapshot(
    ActivityControl* control, ObjectNameResolver* resolver,
    bool treat_global_objects_as_roots) {
  UNIMPLEMENTED(10679);
  return NULL;
}

void HeapProfiler::StartTrackingHeapObjects(bool track_allocations) {
  UNIMPLEMENTED(10687);
}

void HeapProfiler::AddBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  UNIMPLEMENTED(10721);
}

void HeapProfiler::RemoveBuildEmbedderGraphCallback(
    BuildEmbedderGraphCallback callback, void* data) {
  UNIMPLEMENTED(10727);
}

}
