#include "v8.h"
#include "libplatform/libplatform.h"
#include "jerryscript.h"

#define UNIMPLEMENTED(line) abort()

namespace i = v8::internal;

namespace v8 {
namespace platform {
namespace tracing {

// trace-buffer.cc

TraceBufferChunk::TraceBufferChunk(uint32_t seq) : seq_(seq) {
  UNIMPLEMENTED(88);
}

void TraceBufferChunk::Reset(uint32_t new_seq) {
  UNIMPLEMENTED(90);
}

TraceObject* TraceBufferChunk::AddTraceEvent(size_t* event_index) {
  UNIMPLEMENTED(95);
  return NULL;
}

// trace-config.cc

void TraceConfig::AddIncludedCategory(const char* included_category) {
  UNIMPLEMENTED(35);
}

// trace-object.cc

void TraceObject::Initialize(
    char phase, const uint8_t* category_enabled_flag, const char* name,
    const char* scope, uint64_t id, uint64_t bind_id, int num_args,
    const char** arg_names, const uint8_t* arg_types,
    const uint64_t* arg_values,
    std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
    unsigned int flags, int64_t timestamp, int64_t cpu_timestamp) {
  UNIMPLEMENTED(33);
}

TraceObject::~TraceObject() {
  UNIMPLEMENTED(103);
}

// trace-writer.cc

TraceWriter* TraceWriter::CreateJSONTraceWriter(std::ostream& stream) {
  UNIMPLEMENTED(184);
  return NULL;
}

TraceWriter* TraceWriter::CreateJSONTraceWriter(std::ostream& stream,
                                                const std::string& tag) {
  UNIMPLEMENTED(188);
  return NULL;
}

// tracing-controller.cc

TracingController::TracingController() {
  UNIMPLEMENTED(64);
}

TracingController::~TracingController() {
  UNIMPLEMENTED(66);
}

void TracingController::Initialize(TraceBuffer* trace_buffer) {
  UNIMPLEMENTED(81);
}

int64_t TracingController::CurrentTimestampMicroseconds() {
  UNIMPLEMENTED(100);
  return 0;
}

int64_t TracingController::CurrentCpuTimestampMicroseconds() {
  UNIMPLEMENTED(104);
  return 0;
}

uint64_t TracingController::AddTraceEvent(
    char phase, const uint8_t* category_enabled_flag, const char* name,
    const char* scope, uint64_t id, uint64_t bind_id, int num_args,
    const char** arg_names, const uint8_t* arg_types,
    const uint64_t* arg_values,
    std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
    unsigned int flags) {
  UNIMPLEMENTED(162);
  return 0;
}

uint64_t TracingController::AddTraceEventWithTimestamp(
    char phase, const uint8_t* category_enabled_flag, const char* name,
    const char* scope, uint64_t id, uint64_t bind_id, int num_args,
    const char** arg_names, const uint8_t* arg_types,
    const uint64_t* arg_values,
    std::unique_ptr<v8::ConvertableToTraceFormat>* arg_convertables,
    unsigned int flags, int64_t timestamp) {
  UNIMPLEMENTED(176);
  return 0;
}

void TracingController::UpdateTraceEventDuration(
    const uint8_t* category_enabled_flag, const char* name, uint64_t handle) {
  UNIMPLEMENTED(238);
}

void TracingController::StartTracing(TraceConfig* trace_config) {
  UNIMPLEMENTED(279);
}

void TracingController::StopTracing() {
  UNIMPLEMENTED(316);
}

const uint8_t* TracingController::GetCategoryGroupEnabled(
    const char* category_group) {
  UNIMPLEMENTED(386);
  return NULL;
}

void TracingController::AddTraceStateObserver(
    v8::TracingController::TraceStateObserver* observer) {
  UNIMPLEMENTED(437);
}

void TracingController::RemoveTraceStateObserver(
    v8::TracingController::TraceStateObserver* observer) {
  UNIMPLEMENTED(448);
}

}

// default-platform.cc

std::unique_ptr<v8::Platform> NewDefaultPlatform(
    int thread_pool_size, IdleTaskSupport idle_task_support,
    InProcessStackDumping in_process_stack_dumping,
    std::unique_ptr<v8::TracingController> tracing_controller) {
  UNIMPLEMENTED(34);
  return NULL;
}

}

// v8threads.cc

void Locker::Initialize(v8::Isolate* isolate) {
  UNIMPLEMENTED(27);
}

Locker::~Locker() {
  UNIMPLEMENTED(58);
}

// v8.cc

double Platform::SystemClockTimeMillis() {
  UNIMPLEMENTED(163);
}

}
