#include "v8.h"
#include "v8-profiler.h"
#include "jerryscript.h"

#include <signal.h>

#define UNIMPLEMENTED(line) abort()

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
  UNIMPLEMENTED(928);
}

void ResourceConstraints::ConfigureDefaults(uint64_t physical_memory,
                                            uint64_t virtual_memory_limit) {
  UNIMPLEMENTED(1015);
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
  UNIMPLEMENTED(1123);
  return NULL;
}

void V8::FromJustIsNothing() {
  UNIMPLEMENTED(1132);
}

void V8::ToLocalEmpty() {
  UNIMPLEMENTED(1136);
}

HandleScope::HandleScope(Isolate* isolate) {
  UNIMPLEMENTED(1148);
}

HandleScope::~HandleScope() {
  UNIMPLEMENTED(1170);
}

i::Address* HandleScope::CreateHandle(i::Isolate* isolate, i::Address value) {
  UNIMPLEMENTED(1184);
  return NULL;
}

EscapableHandleScope::EscapableHandleScope(Isolate* v8_isolate) {
  UNIMPLEMENTED(1188);
}

i::Address* EscapableHandleScope::Escape(i::Address* escape_value) {
  UNIMPLEMENTED(1195);
  return NULL;
}

SealHandleScope::SealHandleScope(Isolate* isolate)
    : isolate_(NULL) {
  UNIMPLEMENTED(1214);
}

SealHandleScope::~SealHandleScope() {
  UNIMPLEMENTED(1223);
}

void Context::Enter() {
  UNIMPLEMENTED(1236);
}

void Context::Exit() {
  UNIMPLEMENTED(1246);
}

uint32_t Context::GetNumberOfEmbedderDataFields() {
  UNIMPLEMENTED(1307);
  return 0;
}

v8::Local<v8::Value> Context::SlowGetEmbedderData(int index) {
  UNIMPLEMENTED(1315);
  return Local<v8::Value>();
}

void Context::SetEmbedderData(int index, v8::Local<Value> value) {
  UNIMPLEMENTED(1326);
}

void* Context::SlowGetAlignedPointerFromEmbedderData(int index) {
  UNIMPLEMENTED(1337);
  return NULL;
}

void Context::SetAlignedPointerInEmbedderData(int index, void* value) {
  UNIMPLEMENTED(1349);
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
  UNIMPLEMENTED(3358);
  return false;
}

bool Value::FullIsNull() const {
  UNIMPLEMENTED(3365);
  return false;
}

bool Value::IsTrue() const {
  UNIMPLEMENTED(3372);
  return false;
}

bool Value::IsFalse() const {
  UNIMPLEMENTED(3378);
  return false;
}

bool Value::IsFunction() const {
  UNIMPLEMENTED(3384);
  return false;
}

bool Value::IsName() const {
  UNIMPLEMENTED(3386);
  return false;
}

bool Value::FullIsString() const {
  UNIMPLEMENTED(3388);
  return false;
}

bool Value::IsSymbol() const {
  UNIMPLEMENTED(3394);
  return false;
}

bool Value::IsArray() const {
  UNIMPLEMENTED(3396);
  return false;
}

bool Value::IsArrayBuffer() const {
  UNIMPLEMENTED(3398);
  return false;
}

bool Value::IsArrayBufferView() const {
  UNIMPLEMENTED(3403);
  return false;
}

bool Value::IsDataView() const {
  UNIMPLEMENTED(3422);
  return false;
}

bool Value::IsObject() const { 
  UNIMPLEMENTED(3431);
  return false;
}

bool Value::IsNumber() const {
  UNIMPLEMENTED(3433);
  return false;
}

bool Value::IsBigInt() const {
  UNIMPLEMENTED(3435);
  return false;
}

bool Value::IsProxy() const {
  UNIMPLEMENTED(3437);
  return false;
}

bool Value::IsBoolean() const {
  UNIMPLEMENTED(3460);
  return false;
}

bool Value::IsExternal() const {
  UNIMPLEMENTED(3462);
  return false;
}

bool Value::IsInt32() const {
  UNIMPLEMENTED(3474);
  return false;
}

bool Value::IsUint32() const {
  UNIMPLEMENTED(3483);
  return false;
}

bool Value::IsNativeError() const {
  UNIMPLEMENTED(3494);
  return false;
}

bool Value::IsPromise() const {
  UNIMPLEMENTED(3529);
  return false;
}

bool Value::IsModuleNamespaceObject() const {
  UNIMPLEMENTED(3531);
  return false;
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
  UNIMPLEMENTED(3642);
}

void External::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3649);
}

void v8::Object::CheckCast(Value* that) {
  UNIMPLEMENTED(3654);
}

void v8::Function::CheckCast(Value* that) {
  UNIMPLEMENTED(3660);
}

void v8::Boolean::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3666);
}

void v8::Name::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3672);
}

void v8::String::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3677);
}

void v8::Symbol::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3682);
}

void v8::Private::CheckCast(v8::Data* that) {
  UNIMPLEMENTED(3687);
}

void v8::Number::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3694);
}

void v8::Integer::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3700);
}

void v8::Int32::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3706);
}

void v8::Uint32::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3711);
}

void v8::BigInt::CheckCast(v8::Value* that) {
  UNIMPLEMENTED(3716);
}

void v8::Array::CheckCast(Value* that) {
  UNIMPLEMENTED(3721);
}

void v8::Map::CheckCast(Value* that) {
  UNIMPLEMENTED(3726);
}

void v8::Promise::CheckCast(Value* that) {
  UNIMPLEMENTED(3736);
}

void v8::Promise::Resolver::CheckCast(Value* that) {
  UNIMPLEMENTED(3741);
}

void v8::Proxy::CheckCast(Value* that) {
  UNIMPLEMENTED(3746);
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
  UNIMPLEMENTED(4022);
  return Just(false);
}

Maybe<bool> v8::Object::Set(v8::Local<v8::Context> context, uint32_t index,
                            v8::Local<Value> value) {
  UNIMPLEMENTED(4038);
  return Just(false);
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
  UNIMPLEMENTED(4178);
  return Just(false);
}

Maybe<bool> v8::Object::DefineProperty(v8::Local<v8::Context> context,
                                       v8::Local<Name> key,
                                       PropertyDescriptor& descriptor) {
  UNIMPLEMENTED(4213);
  return Just(false);
}

Maybe<bool> v8::Object::SetPrivate(Local<Context> context, Local<Private> key,
                                   Local<Value> value) {
  UNIMPLEMENTED(4229);
  return Just(false);
}

MaybeLocal<Value> v8::Object::Get(Local<v8::Context> context,
                                  Local<Value> key) {
  UNIMPLEMENTED(4256);
  return MaybeLocal<Value>();
}

MaybeLocal<Value> v8::Object::Get(Local<Context> context, uint32_t index) {
  UNIMPLEMENTED(4268);
  return MaybeLocal<Value>();
}

MaybeLocal<Value> v8::Object::GetPrivate(Local<Context> context,
                                         Local<Private> key) {
  UNIMPLEMENTED(4278);
  return MaybeLocal<Value>();
}

MaybeLocal<Value> v8::Object::GetOwnPropertyDescriptor(Local<Context> context,
                                                       Local<Name> key) {
  UNIMPLEMENTED(4305);
  return MaybeLocal<Value>();
}

Local<Value> v8::Object::GetPrototype() {
  UNIMPLEMENTED(4322);
  return Local<Value>();
}

Maybe<bool> v8::Object::SetPrototype(Local<Context> context,
                                     Local<Value> value) {
  UNIMPLEMENTED(4329);
  return Just(true);
}

MaybeLocal<Array> v8::Object::GetPropertyNames(Local<Context> context) {
  UNIMPLEMENTED(4361);
  return MaybeLocal<Array>();
}

MaybeLocal<Array> v8::Object::GetPropertyNames(
    Local<Context> context, KeyCollectionMode mode,
    PropertyFilter property_filter, IndexFilter index_filter,
    KeyConversionMode key_conversion) {
  UNIMPLEMENTED(4368);
  return MaybeLocal<Array>();
}

MaybeLocal<Array> v8::Object::GetOwnPropertyNames(Local<Context> context) {
  UNIMPLEMENTED(4390);
  return MaybeLocal<Array>();
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
  UNIMPLEMENTED(5656);
}

void v8::V8::ShutdownPlatform() {
  UNIMPLEMENTED(5660);
}

bool v8::V8::Initialize(const int build_config) {
  UNIMPLEMENTED(5662);
  return false;
}

bool TryHandleWebAssemblyTrapPosix(int sig_code, siginfo_t* info,
                                   void* context) {
  UNIMPLEMENTED(5686);
  return false;
}

bool V8::EnableWebAssemblyTrapHandler(bool use_v8_signal_handler) {
  UNIMPLEMENTED(5710);
  return false;
}

void v8::V8::SetEntropySource(EntropySource entropy_source) {
  UNIMPLEMENTED(5726);
}

bool v8::V8::Dispose() {
  UNIMPLEMENTED(5735);
  return false;
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
  UNIMPLEMENTED(5797);
  return NULL;
}

Local<Context> v8::Context::New(
    v8::Isolate* external_isolate, v8::ExtensionConfiguration* extensions,
    v8::MaybeLocal<ObjectTemplate> global_template,
    v8::MaybeLocal<Value> global_object,
    DeserializeInternalFieldsCallback internal_fields_deserializer,
    v8::MicrotaskQueue* microtask_queue) {
  UNIMPLEMENTED(5977);
  return Local<Context>();
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
  UNIMPLEMENTED(6031);
}

Local<Value> v8::Context::GetSecurityToken() {
  UNIMPLEMENTED(6042);
  return Local<Value>();
}

v8::Isolate* Context::GetIsolate() {
  UNIMPLEMENTED(6050);
  return NULL;
}

v8::Local<v8::Object> Context::Global() {
  UNIMPLEMENTED(6055);
  return v8::Local<v8::Object>();
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
  UNIMPLEMENTED(6320);
  return MaybeLocal<String>();
}

MaybeLocal<String> String::NewFromOneByte(Isolate* isolate, const uint8_t* data,
                                          NewStringType type, int length) {
  UNIMPLEMENTED(6326);
  return MaybeLocal<String>();
}

MaybeLocal<String> String::NewFromTwoByte(Isolate* isolate,
                                          const uint16_t* data,
                                          NewStringType type, int length) {
  UNIMPLEMENTED(6332);
  return MaybeLocal<v8::String>();
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
  UNIMPLEMENTED(6475);
  return NULL;
}

Local<v8::Object> v8::Object::New(Isolate* isolate) {
  UNIMPLEMENTED(6480);
  return Local<v8::Object>();
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

CompiledWasmModule WasmModuleObject::GetCompiledModule() {
  UNIMPLEMENTED(7211);
  return Utils::Convert(NULL);
}

MaybeLocal<WasmModuleObject> WasmModuleObject::FromCompiledModule(
    Isolate* isolate, const CompiledWasmModule& compiled_module) {
  UNIMPLEMENTED(7217);
  return MaybeLocal<WasmModuleObject>();
}

void* v8::ArrayBuffer::Allocator::Reallocate(void* data, size_t old_length,
                                             size_t new_length) {
  UNIMPLEMENTED(7241);
  return NULL;
}

v8::ArrayBuffer::Allocator* v8::ArrayBuffer::Allocator::NewDefaultAllocator() {
  UNIMPLEMENTED(7257);
  return NULL;
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
  UNIMPLEMENTED(7852);
  return Local<Symbol>();
}

Local<Private> v8::Private::New(Isolate* isolate, Local<String> name) {
  UNIMPLEMENTED(7899);
  return Local<Private>();
}

Local<Private> v8::Private::ForApi(Isolate* isolate, Local<String> name) {
  UNIMPLEMENTED(7909);
  return Local<Private>();
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
  UNIMPLEMENTED(7998);
}

void Isolate::CheckMemoryPressure() {
  UNIMPLEMENTED(8004);
}

HeapProfiler* Isolate::GetHeapProfiler() {
  UNIMPLEMENTED(8010);
  return NULL;
}

void Isolate::SetIdle(bool is_idle) {
  UNIMPLEMENTED(8016);
}

ArrayBuffer::Allocator* Isolate::GetArrayBufferAllocator() {
  UNIMPLEMENTED(8021);
  return NULL;
}

bool Isolate::InContext() {
  UNIMPLEMENTED(8026);
  return false;
}

void Isolate::ClearKeptObjects() {
  UNIMPLEMENTED(8031);
}

v8::Local<v8::Context> Isolate::GetCurrentContext() {
  UNIMPLEMENTED(8036);
  return v8::Local<v8::Context>();
}

v8::Local<Value> Isolate::ThrowException(v8::Local<v8::Value> value) {
  UNIMPLEMENTED(8068);
  return v8::Undefined(NULL);
}

void Isolate::AddGCPrologueCallback(GCCallbackWithData callback, void* data,
                                    GCType gc_type) {
  UNIMPLEMENTED(8081);
}

void Isolate::RemoveGCPrologueCallback(GCCallbackWithData callback,
                                       void* data) {
  UNIMPLEMENTED(8087);
}

void Isolate::AddGCEpilogueCallback(GCCallbackWithData callback, void* data,
                                    GCType gc_type) {
  UNIMPLEMENTED(8093);
}

void Isolate::RemoveGCEpilogueCallback(GCCallbackWithData callback,
                                       void* data) {
  UNIMPLEMENTED(8099);
}

void Isolate::TerminateExecution() {
  UNIMPLEMENTED(8146);
}

void Isolate::CancelTerminateExecution() {
  UNIMPLEMENTED(8156);
}

void Isolate::RequestInterrupt(InterruptCallback callback, void* data) {
  UNIMPLEMENTED(8162);
}

void Isolate::RequestGarbageCollectionForTesting(GarbageCollectionType type) {
  UNIMPLEMENTED(8167);
}

Isolate* Isolate::GetCurrent() {
  UNIMPLEMENTED(8181);
  return NULL;
}

Isolate* Isolate::Allocate() {
  UNIMPLEMENTED(8187);
}

void Isolate::Initialize(Isolate* isolate,
                         const v8::Isolate::CreateParams& params) {
  UNIMPLEMENTED(8193);
}

Isolate* Isolate::New(const Isolate::CreateParams& params) {
  UNIMPLEMENTED(8273);
  return NULL;
}

void Isolate::Dispose() {
  UNIMPLEMENTED(8279);
}

void Isolate::Enter() {
  UNIMPLEMENTED(8298);
}

void Isolate::Exit() {
  UNIMPLEMENTED(8303);
}

void Isolate::SetAbortOnUncaughtExceptionCallback(
    AbortOnUncaughtExceptionCallback callback) {
  UNIMPLEMENTED(8308);
}

void Isolate::SetHostImportModuleDynamicallyCallback(
    HostImportModuleDynamicallyCallback callback) {
  UNIMPLEMENTED(8336);
}

void Isolate::SetHostInitializeImportMetaObjectCallback(
    HostInitializeImportMetaObjectCallback callback) {
  UNIMPLEMENTED(8342);
}

void Isolate::SetPrepareStackTraceCallback(PrepareStackTraceCallback callback) {
  UNIMPLEMENTED(8348);
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
  UNIMPLEMENTED(8638);
}

void Isolate::SetPromiseRejectCallback(PromiseRejectCallback callback) {
  UNIMPLEMENTED(8643);
}

void Isolate::EnqueueMicrotask(Local<Function> v8_function) {
  UNIMPLEMENTED(8655);
}

void Isolate::SetMicrotasksPolicy(MicrotasksPolicy policy) {
  UNIMPLEMENTED(8675);
}

void Isolate::LowMemoryNotification() {
  UNIMPLEMENTED(8759);
}

void Isolate::SetStackLimit(uintptr_t stack_limit) {
  UNIMPLEMENTED(8838);
}

void Isolate::AddNearHeapLimitCallback(v8::NearHeapLimitCallback callback,
                                       void* data) {
  UNIMPLEMENTED(8949);
}

bool Isolate::AddMessageListenerWithErrorLevel(MessageCallback that,
                                               int message_levels,
                                               Local<Value> data) {
  UNIMPLEMENTED(8977);
  return false;
}

void Isolate::SetCaptureStackTraceForUncaughtExceptions(
    bool capture, int frame_limit, StackTrace::StackTraceOptions options) {
  UNIMPLEMENTED(9018);
}

void v8::Isolate::DateTimeConfigurationChangeNotification(
    TimeZoneDetection time_zone_detection) {
  UNIMPLEMENTED(9052);
}

void MicrotasksScope::PerformCheckpoint(Isolate* v8_isolate) {
  UNIMPLEMENTED(9122);
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
  UNIMPLEMENTED(10452);
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
