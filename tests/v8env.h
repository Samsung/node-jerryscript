#ifndef TEST_V8_H_
#define TEST_V8_H_

#include "libplatform/libplatform.h"
#include "v8.h"

class V8Scopes {
public:
    V8Scopes(v8::Isolate* isolate)
        : m_isolateScope(isolate)
        , m_handleScope(isolate)
        , m_context(v8::Context::New(isolate))
        , m_contexScope(m_context)
    {
    }

    v8::Local<v8::Context> getContext() { return m_context; }

private:
    v8::Isolate::Scope m_isolateScope;
    v8::HandleScope m_handleScope;
    v8::Local<v8::Context> m_context;
    v8::Context::Scope m_contexScope;
};


class V8Environment {
public:
    V8Environment(int argc, char** argv)
        : m_platform(BuildPlatform(argc, argv))
        , m_create_params(BuildCreateParams())
        , m_isolate(v8::Isolate::New(m_create_params))
        , m_scopes(new V8Scopes(m_isolate))
    {
    }

    ~V8Environment() {
        delete m_scopes;

        // Dispose the isolate and tear down V8.
        m_isolate->Dispose();
        // No need to call delete on the isolate,
        // it will delete itself and the destructor is deleted in the v8 header.

        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
    }

    v8::Isolate* getIsolate() { return m_isolate; }
    v8::Local<v8::Context> getContext() { return m_scopes->getContext(); }

private:
    static std::unique_ptr<v8::Platform> BuildPlatform(int argc, char** argv) {
        v8::V8::InitializeICUDefaultLocation(argv[0]);
        v8::V8::InitializeExternalStartupData(argv[0]);
        //std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
        std::unique_ptr<v8::Platform> platform(v8::platform::CreateDefaultPlatform());
        v8::V8::InitializePlatform(platform.get());
        v8::V8::Initialize();

        return platform;
    }

    static v8::Isolate::CreateParams BuildCreateParams(void) {
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        return create_params;
    }

    std::unique_ptr<v8::Platform> m_platform;
    v8::Isolate::CreateParams m_create_params;
    v8::Isolate* m_isolate;
    V8Scopes* m_scopes;
};

#endif // TEST_V8_H_
