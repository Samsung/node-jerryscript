#include "v8.h"
#include "v8-inspector.h"
#include "jerryscript.h"

#define UNIMPLEMENTED(line) abort()

namespace i = v8::internal;

namespace v8_inspector {

// string-util.cc

std::unique_ptr<StringBuffer> StringBuffer::create(StringView string) {
  UNIMPLEMENTED(163);
  return NULL;
}

// v8-inspector-impl.cc

std::unique_ptr<V8Inspector> V8Inspector::create(v8::Isolate* isolate,
                                                 V8InspectorClient* client) {
  UNIMPLEMENTED(52);
  return NULL;
}

// v8-inspector-session-impl.cc

bool V8InspectorSession::canDispatchMethod(StringView method) {
  UNIMPLEMENTED(65);
  return false;
}

}
