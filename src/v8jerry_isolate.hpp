#ifndef V8JERRY_ISOLATE_HPP
#define V8JERRY_ISOLATE_HPP

#include <deque>
#include <vector>

#include <v8.h>

#include "jerryscript.h"
#include "jerryscript-port-default.h"

#include "v8jerry_value.hpp"

class JerryContext;
class JerryHandle;
class JerryHandleScope;
enum JerryHandleScopeType : int;
class JerryTemplate;

class JerryIsolate {
public:
    JerryIsolate(const v8::Isolate::CreateParams& params);
    ~JerryIsolate() {}

    const JerryValue& HelperMapNew(void) const { return *m_fn_map_new; }
    const JerryValue& HelperMapSet(void) const { return *m_fn_map_set; }
    const JerryValue& HelperObjectAssign(void) const { return *m_fn_object_assign; }
    const JerryValue& HelperIsMap(void) const { return *m_fn_is_map; }
    const JerryValue& HelperIsSet(void) const { return *m_fn_is_set; }

    void Enter(void);
    void Exit(void);

    void Dispose(void);

    void PushContext(JerryContext* context);
    void PopContext(JerryContext* context);
    JerryContext* CurrentContext(void);

    void PushHandleScope(JerryHandleScopeType type, void* handle_scope);
    void PopHandleScope(void* handle_scope);
    JerryHandleScope* CurrentHandleScope(void);

    void AddToHandleScope(JerryHandle* jvalue);
    void EscapeHandle(JerryHandle* jvalue);
    void SealHandleScope(void* handle_scope);

    void AddTemplate(JerryTemplate* handle);

    void SetFatalErrorHandler(v8::FatalErrorCallback callback) { m_fatalErrorCallback = callback; }
    void ReportFatalError(const char* location, const char* message);

    static v8::Isolate* toV8(JerryIsolate* iso) { return reinterpret_cast<v8::Isolate*>(iso); }
    static JerryIsolate* fromV8(v8::Isolate* iso) { return reinterpret_cast<JerryIsolate*>(iso); }
    static JerryIsolate* fromV8(v8::internal::Isolate* iso) { return reinterpret_cast<JerryIsolate*>(iso); }

    static JerryIsolate* GetCurrent(void);

private:
    void InitalizeSlots(void);

    // Slots accessed by v8::Isolate::Get/SetData
    // They must be the first field of GraalIsolate
    void* m_slot[22] = {};

    std::deque<JerryHandleScope*> m_handleScopes;
    std::deque<JerryContext*> m_contexts;
    std::vector<JerryTemplate*> m_templates;
    JerryValue* m_fn_map_new;
    JerryValue* m_fn_is_map;
    JerryValue* m_fn_is_set;
    JerryValue* m_fn_map_set;
    JerryValue* m_fn_object_assign;

    v8::FatalErrorCallback m_fatalErrorCallback;

    static JerryIsolate* s_currentIsolate;
};

#endif /* V8JERRY_ISOLATE_HPP */

