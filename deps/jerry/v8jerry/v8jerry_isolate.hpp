#ifndef V8JERRY_ISOLATE_HPP
#define V8JERRY_ISOLATE_HPP

#include <deque>
#include <vector>
#include <unordered_map>

#include <v8.h>

#include "jerryscript.h"
#include "jerryscript-port-default.h"

#include "v8jerry_allocator.hpp"
#include "v8jerry_atomics.hpp"
#include "v8jerry_value.hpp"
#include "v8jerry_utils.hpp"

#ifdef __POSIX__
#include <pthread.h>
#endif

class JerryHandle;
class JerryHandleScope;
enum JerryHandleScopeType : int;
class JerryTemplate;
class JerryObjectTemplate;

class JerryIsolate {
public:
    JerryIsolate();
    JerryIsolate(const v8::Isolate::CreateParams& params);
    ~JerryIsolate() {}

    void InitializeJerryIsolate(const v8::Isolate::CreateParams& params);

    const JerryPolyfill& HelperMapSet(void) const { return *m_fn_map_set; }
    const JerryPolyfill& HelperSetAdd(void) const { return *m_fn_set_add; }
    const JerryPolyfill& HelperObjectAssign(void) const { return *m_fn_object_assign; }
    const JerryPolyfill& HelperConversionFailer(void) const { return *m_fn_conversion_failer; }
    const JerryPolyfill& HelperGetOwnPropDesc(void) const { return *m_fn_get_own_prop; }
    const JerryPolyfill& HelperGetOwnPropNames(void) const { return *m_fn_get_own_names; }
    const JerryPolyfill& HelperGetPropNames(void) const { return *m_fn_get_names; }
    const JerryPolyfill& HelperSetIntegrityLevel(void) const { return *m_fn_set_integrity; }

    void Enter(void);
    void Exit(void);
    bool IsTerminated(void) const { return m_terminated; }
    void Terminate(void);
    void CancelTerminate(void);

    void Dispose(void);

    void PushTryCatch(void* try_catch_obj);
    void PopTryCatch(void* try_catch_obj);
    void SetErrorVerbose(bool value);
    void SetError(const jerry_value_t error_value);
    void ClearError(void);
    bool HasError(void);
    void TryReportError(void);
    JerryValue* GetRawError(void) { return m_current_error; }

    void PushContext(JerryValue* context);
    void PopContext(JerryValue* context);
    JerryValue* CurrentContext(void);
    JerryValue* GetGlobalSymbol(JerryValue* name);

    void PushHandleScope(JerryHandleScopeType type, void* handle_scope);
    void PopHandleScope(void* handle_scope);
    JerryHandleScope* CurrentHandleScope(void);

    void AddToHandleScope(JerryHandle* jvalue);
    void EscapeHandle(JerryHandle* jvalue);
    void SealHandleScope(void* handle_scope);

    void AddTemplate(JerryTemplate* handle);

    void AddMessageListener(v8::MessageCallback callback) { m_messageCallback = callback; }
    void ReportMessage(v8::Local<v8::Message> message, v8::Local<v8::Value> error);

    void SetFatalErrorHandler(v8::FatalErrorCallback callback) { m_fatalErrorCallback = callback; }
    void ReportFatalError(const char* location, const char* message);

    void EnqueueMicrotask(JerryValue* func);
    void RunMicrotasks(void);

    void SetPromiseHook(v8::PromiseHook promise_hook) { m_promise_hook = promise_hook; }
    void SetPromiseRejectCallback(v8::PromiseRejectCallback callback) { m_promise_reject_calback = callback; }

    void SetEternal(JerryValue* value, int* index);
    void ClearEternal(JerryValue* value);
    bool IsEternal(JerryValue* value);

    void AddUTF16String(std::u16string*);
    void RemoveUTF16String(uint16_t*);

    void FormatError(const JerryValue& error, std::ostream& out);
    void UpdateErrorStackProp(JerryValue& error);

    JerryValue* Undefined() {
        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiSystemPointerSize;
        return reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex]);
    }

    JerryObjectTemplate* HiddenObjectTemplate(void);

    static v8::Isolate* toV8(JerryIsolate* iso) { return reinterpret_cast<v8::Isolate*>(iso); }
    static JerryIsolate* fromV8(v8::Isolate* iso) { return reinterpret_cast<JerryIsolate*>(iso); }
    static JerryIsolate* fromV8(v8::internal::Isolate* iso) { return reinterpret_cast<JerryIsolate*>(iso); }

    static JerryIsolate* GetCurrent(void);

private:
    void SetError(JerryValue* error);
    void InitalizeSlots(void);

    void InitializeSharedArrayBuffer();
    void InitializeAtomics();

    // Slots accessed by v8::Isolate::Get/SetData
    // They must be the first field of GraalIsolate
    void* m_slot[26] = {};

    // TODO: support non POSIX systems.
    #ifdef __POSIX__
        pthread_mutex_t m_lock;
    #endif

    v8::Locker* m_lock_owner;
    friend class v8::Locker;

    std::deque<JerryHandleScope*> m_handleScopes;
    std::deque<JerryValue*> m_contexts;
    std::vector<JerryTemplate*> m_templates;
    std::vector<JerryValue*> m_eternals;
    std::vector<std::pair<jerry_value_t, jerry_value_t>> m_global_symbols;
    std::unordered_map<uint16_t*, std::u16string*> m_utf16strs;

    JerryObjectTemplate* m_hidden_object_template;

    JerryPolyfill* m_fn_map_set;
    JerryPolyfill* m_fn_set_add;
    JerryPolyfill* m_fn_object_assign;
    JerryPolyfill* m_fn_conversion_failer;
    JerryPolyfill* m_fn_get_own_prop;
    JerryPolyfill* m_fn_get_own_names;
    JerryPolyfill* m_fn_get_names;
    JerryPolyfill* m_fn_set_integrity;

    JerryValue* m_magic_string_stack;

    int m_try_catch_count;
    JerryValue* m_current_error;

    v8::FatalErrorCallback m_fatalErrorCallback;
    v8::MessageCallback m_messageCallback;

    bool m_terminated;
    bool m_autorun_tasks;

    std::vector<JerryValue*> m_micro_tasks;

    v8::PromiseHook m_promise_hook;
    v8::PromiseRejectCallback m_promise_reject_calback;

    static JerryIsolate* s_currentIsolate;
};

#endif /* V8JERRY_ISOLATE_HPP */
