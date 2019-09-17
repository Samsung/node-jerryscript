#ifndef V8JERRY_ISOLATE_HPP
#define V8JERRY_ISOLATE_HPP

#include <deque>
#include <vector>

#include <v8.h>

#include "jerryscript.h"
#include "jerryscript-port-default.h"

#include "v8jerry_value.hpp"
#include "v8jerry_utils.hpp"

class JerryContext;
class JerryHandle;
class JerryHandleScope;
enum JerryHandleScopeType : int;
class JerryTemplate;

class JerryIsolate {
public:
    JerryIsolate(const v8::Isolate::CreateParams& params);
    ~JerryIsolate() {}

    const JerryPolyfill& HelperMapNew(void) const { return *m_fn_map_new; }
    const JerryPolyfill& HelperMapSet(void) const { return *m_fn_map_set; }
    const JerryPolyfill& HelperObjectAssign(void) const { return *m_fn_object_assign; }
    const JerryPolyfill& HelperIsMap(void) const { return *m_fn_is_map; }
    const JerryPolyfill& HelperIsSet(void) const { return *m_fn_is_set; }
    const JerryPolyfill& HelperConversionFailer(void) const { return *m_fn_conversion_failer; }

    void Enter(void);
    void Exit(void);

    void Dispose(void);

    void PushTryCatch(void* try_catch_obj);
    void PopTryCatch(void* try_catch_obj);
    void SetError(JerryValue* error);
    void SetError(const jerry_value_t error_value);
    void ClearError(void);
    bool HasError(void);

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
    JerryPolyfill* m_fn_map_new;
    JerryPolyfill* m_fn_is_map;
    JerryPolyfill* m_fn_is_set;
    JerryPolyfill* m_fn_map_set;
    JerryPolyfill* m_fn_object_assign;
    JerryPolyfill* m_fn_conversion_failer;

    JerryValue* m_magic_string_stack;

    int m_try_catch_count;
    JerryValue* m_current_error;

    v8::FatalErrorCallback m_fatalErrorCallback;

    static JerryIsolate* s_currentIsolate;
};

#endif /* V8JERRY_ISOLATE_HPP */

