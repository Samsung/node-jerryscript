#ifndef TEST_V8_H_
#define TEST_V8_H_

#include "libplatform/libplatform.h"
#include "v8.h"

class V8Scopes {
public:
  V8Scopes(v8::Isolate*);
  v8::Local<v8::Context> getContext() { return m_context; }

private:
  v8::Isolate::Scope m_isolateScope;
  v8::HandleScope m_handleScope;
  v8::Local<v8::Context> m_context;
  v8::Context::Scope m_contexScope;
};

V8Scopes::V8Scopes(v8::Isolate* isolate)
  : m_isolateScope(isolate)
  , m_handleScope(isolate)
  , m_context(v8::Context::New(isolate))
  , m_contexScope(m_context)
{
}

class V8Environment {
public:
  V8Environment(int, char**);
  ~V8Environment();

  v8::Isolate* getIsolate() { return m_isolate; }
  v8::Local<v8::Context> getContext() { return m_scopes->getContext(); }

private:
  v8::Isolate* m_isolate;
  V8Scopes* m_scopes;
};

static v8::Isolate* BuildIsolate(int argc, char** argv) {
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  //std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  std::unique_ptr<v8::Platform> platform(v8::platform::CreateDefaultPlatform());
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // Dummy structure.
  v8::Isolate::CreateParams params;
  // Create a new Isolate and make it the current one.
  return v8::Isolate::New(params);
}

V8Environment::V8Environment(int argc, char** argv)
  : m_isolate(BuildIsolate(argc, argv))
  , m_scopes(new V8Scopes(m_isolate))
{
}

V8Environment::~V8Environment() {
  // Dispose the isolate and tear down V8.
  delete m_scopes;
  m_isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
}

#endif // TEST_V8_H_
