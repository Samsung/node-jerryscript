#ifndef V8JERRY_CONTEXT_HPP
#define V8JERRY_CONTEXT_HPP

namespace v8 {
    class Context;
};

#include "v8jerry_value.hpp"
#include "v8jerry_isolate.hpp"

class JerryContext : public JerryHandle {
public:

    JerryContext(JerryIsolate* iso)
        : JerryHandle(JerryHandle::Context)
        , m_isolate(iso)
    {}

    JerryIsolate* GetIsolate(void) {
        return m_isolate;
    }

    void Enter(void) {
        m_isolate->Enter();
        m_isolate->PushContext(this);
    }

    void Exit(void) {
        m_isolate->PopContext(this);
        m_isolate->Exit();
    }

    static v8::Context* toV8(JerryContext* ctx) {
        return reinterpret_cast<v8::Context*>(ctx);
    }

    static JerryContext* fromV8(v8::Context* ctx) {
        return reinterpret_cast<JerryContext*>(ctx);
    }

private:
    JerryIsolate* m_isolate;
};

#endif /* V8JERRY_CONTEXT_HPP */
