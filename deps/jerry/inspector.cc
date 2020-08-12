#include "v8.h"
#include "v8-inspector.h"
#include "jerryscript.h"

#define UNIMPLEMENTED(line) abort()

namespace i = v8::internal;

namespace v8_inspector {

// string-util.cc

namespace {
// An empty string buffer doesn't own any string data; its ::string() returns a
// default-constructed StringView instance.
class EmptyStringBuffer : public StringBuffer {
 public:
  StringView string() const override { return StringView(); }
};

// Contains LATIN1 text data or CBOR encoded binary data in a vector.
class StringBuffer8 : public StringBuffer {
 public:
  explicit StringBuffer8(std::vector<uint8_t> data) : data_(std::move(data)) {}

  StringView string() const override {
    return StringView(data_.data(), data_.size());
  }

 private:
  std::vector<uint8_t> data_;
};

// Contains a 16 bit string (String16).
class StringBuffer16 : public StringBuffer {
 public:
  explicit StringBuffer16(std::vector<uint16_t> data) : data_(std::move(data)) {}

  StringView string() const override {
    return StringView(data_.data(), data_.size());
  }

 private:
  std::vector<uint16_t> data_;
};
}  // namespace

std::unique_ptr<StringBuffer> StringBuffer::create(StringView string) {
  if (string.length() == 0) return std::make_unique<EmptyStringBuffer>();
  if (string.is8Bit()) {
    return std::make_unique<StringBuffer8>(std::vector<uint8_t>(
        string.characters8(), string.characters8() + string.length()));
  }
  return std::make_unique<StringBuffer16>(std::vector<uint16_t>(
        string.characters16(), string.characters16() + string.length()));
  return NULL;
}

// v8-inspector-impl.cc

class JerryScriptInspector: public V8Inspector {
  virtual ~JerryScriptInspector() {}

  // Contexts instrumentation.
  virtual void contextCreated(const V8ContextInfo&) {}
  virtual void contextDestroyed(v8::Local<v8::Context>) {}
  virtual void resetContextGroup(int contextGroupId) {}
  virtual v8::MaybeLocal<v8::Context> contextById(int contextId) {}

  // Various instrumentation.
  virtual void idleStarted() {}
  virtual void idleFinished() {}

  // Async stack traces instrumentation.
  virtual void asyncTaskScheduled(StringView taskName, void* task,
                                  bool recurring) {
  }
  virtual void asyncTaskCanceled(void* task) {}
  virtual void asyncTaskStarted(void* task) {}
  virtual void asyncTaskFinished(void* task) {}
  virtual void allAsyncTasksCanceled() {}

  virtual V8StackTraceId storeCurrentStackTrace(StringView description) {}
  virtual void externalAsyncTaskStarted(const V8StackTraceId& parent) {}
  virtual void externalAsyncTaskFinished(const V8StackTraceId& parent) {}

  // Exceptions instrumentation.
  virtual unsigned exceptionThrown(v8::Local<v8::Context>, StringView message,
                                   v8::Local<v8::Value> exception,
                                   StringView detailedMessage, StringView url,
                                   unsigned lineNumber, unsigned columnNumber,
                                   std::unique_ptr<V8StackTrace>,
                                   int scriptId) {
  }
  virtual void exceptionRevoked(v8::Local<v8::Context>, unsigned exceptionId,
                                StringView message) {
  }

  virtual std::unique_ptr<V8InspectorSession> connect(int contextGroupId,
                                                      Channel*,
                                                      StringView state) {
  }

  // API methods.
  virtual std::unique_ptr<V8StackTrace> createStackTrace(
      v8::Local<v8::StackTrace>) {
  }
  virtual std::unique_ptr<V8StackTrace> captureStackTrace(bool fullStack) {
  }

  virtual std::shared_ptr<Counters> enableCounters() {}
};

std::unique_ptr<V8Inspector> V8Inspector::create(v8::Isolate* isolate,
                                                 V8InspectorClient* client) {
  return std::unique_ptr<V8Inspector>(new JerryScriptInspector());
}

// v8-inspector-session-impl.cc

bool V8InspectorSession::canDispatchMethod(StringView method) {
  UNIMPLEMENTED(65);
  return false;
}

}
