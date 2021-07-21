#ifndef V8JERRY_ISOLATE_HPP
#define V8JERRY_ISOLATE_HPP

#include <deque>
#include <vector>
#include <unordered_map>

#include <v8.h>

#include "jerryscript.h"
#include "jerryscript-port-default.h"

#include "v8jerry_allocator.hpp"
#include "v8jerry_value.hpp"
#include "v8jerry_utils.hpp"

#ifdef __POSIX__
#include <pthread.h>
#endif

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
    const JerryPolyfill& HelperSetIntegrityLevel(void) const { return *m_fn_set_integrity; }

    JerryValue *MagicStringStack() { return m_magic_string_stack; }
    JerryValue *CallSitePrototype() { return m_call_site_prototype; }

    void Enter(void);
    void Exit(void);
    bool IsTerminated(void) const { return m_terminated; }
    void Terminate(void);
    void CancelTerminate(void);

    void Dispose(void);

    v8::TryCatch* GetLastTryCatch() { return m_last_try_catch; }
    void SetLastTryCatch(v8::TryCatch* try_catch) { m_last_try_catch = try_catch; }
    bool HasError(void) { return m_has_current_error; }
    jerry_value_t GetError(void) { return m_current_error; }
    JerryValue* GetGlobalError(void) { return m_global_error; }
    void SetError(const jerry_value_t error_value);
    void RestoreError(JerryValue* error);
    void RestoreError(jerry_value_t error);
    void SetGlobalError(JerryValue* error) { m_global_error = error; }
    void ProcessError(bool is_verbose);
    void SetErrorVerbose(bool value);
    void IncTryDepth() { m_try_depth++; }
    void DecTryDepth() { m_try_depth--; }

    jerry_value_t TakeError(void) {
        m_has_current_error = false;
        return m_current_error;
    }

    JerryValue *TakeGlobalError(void) {
        JerryValue *global_error = m_global_error;
        m_global_error = NULL;
        return global_error;
    }

    void PushContext(JerryValue* context);
    void PopContext();
    JerryValue* CurrentContext(void);
    JerryValue** GetLastReturnValue() { return m_last_return_value; }
    void SetLastReturnValue(JerryValue** return_value) { m_last_return_value = return_value; }
    JerryValue* GetGlobalSymbol(JerryValue* name);

    void PushHandleScope(JerryHandleScopeType type);
    void PopHandleScope();
    JerryHandleScope* CurrentHandleScope(void);

    void AddToHandleScope(JerryValue* jvalue);
    void EscapeHandle(JerryValue* jvalue);
    void SealHandleScope(void* handle_scope);

    void AddTemplate(JerryTemplate* handle);

    void AddMessageListener(v8::MessageCallback callback) { m_messageCallback = callback; }
    void ReportMessage(v8::Local<v8::Message> message, v8::Local<v8::Value> error);

    void SetFatalErrorHandler(v8::FatalErrorCallback callback) { m_fatalErrorCallback = callback; }
    void ReportFatalError(const char* location, const char* message);
    void SetAbortOnUncaughtExceptionCallback(v8::Isolate::AbortOnUncaughtExceptionCallback callback) {
        m_abortOnUncaughtExceptionCallback = callback;
    }

    void EnqueueMicrotask(JerryValue* func);
    void RunMicrotasks(void);

    v8::PromiseHook GetPromiseHook() { return m_promise_hook; }
    void SetPromiseHook(v8::PromiseHook promise_hook) {
        m_promise_hook = promise_hook;
        UpdatePromiseFilters();
    }
    v8::PromiseRejectCallback GetPromiseRejectCallback() { return m_promise_reject_callback; }
    void SetPromiseRejectCallback(v8::PromiseRejectCallback callback) {
        m_promise_reject_callback = callback;
        UpdatePromiseFilters();
    }

    v8::PrepareStackTraceCallback PrepareStackTraceCallback() {
        return m_prepareStackTraceCallback;
    }
    void SetPrepareStackTraceCallback(v8::PrepareStackTraceCallback callback) {
        m_prepareStackTraceCallback = callback;
    }
    v8::HostImportModuleDynamicallyCallback HostImportModuleDynamicallyCallback() {
        return m_importModuleDynamicallyCallback;
    }
    void SetHostImportModuleDynamicallyCallback(v8::HostImportModuleDynamicallyCallback callback) {
        m_importModuleDynamicallyCallback = callback;
    }

    void SetEternal(JerryValue* value, int* index);
    void ClearEternal(JerryValue* value);
    bool IsEternal(JerryValue* value);

    void AddUTF16String(std::u16string*);
    void RemoveUTF16String(uint16_t*);

    JerryValue* Undefined() {
        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiSystemPointerSize;
        return reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kUndefinedValueRootIndex]);
    }

    JerryValue* Hole() {
        int root_offset = v8::internal::Internals::kIsolateRootsOffset / v8::internal::kApiSystemPointerSize;
        return reinterpret_cast<JerryValue*>(m_slot[root_offset + v8::internal::Internals::kTheHoleValueRootIndex]);
    }

    JerryObjectTemplate* HiddenObjectTemplate(void);
    static jerry_value_t GetModuleException(jerry_value_t module);

    static v8::Isolate* toV8(JerryIsolate* iso) { return reinterpret_cast<v8::Isolate*>(iso); }
    static JerryIsolate* fromV8(v8::Isolate* iso) { return reinterpret_cast<JerryIsolate*>(iso); }
    static JerryIsolate* fromV8(v8::internal::Isolate* iso) { return reinterpret_cast<JerryIsolate*>(iso); }

    static JerryIsolate* GetCurrent(void);

private:
    void InitalizeSlots(void);

    void InitializeSharedArrayBuffer();
    void InitializeAtomics();

    void UpdatePromiseFilters(void);

    // Slots accessed by v8::Isolate::Get/SetData
    // They must be the first field of GraalIsolate
    void* m_slot[26] = {};

    // TODO: support non POSIX systems.
    #ifdef __POSIX__
        pthread_mutex_t m_lock;
    #endif

    v8::Locker* m_lock_owner;
    friend class v8::Locker;

    std::deque<std::pair<JerryValue*, jerry_value_t>> m_contexts;
    std::vector<JerryTemplate*> m_templates;
    std::vector<JerryValue*> m_eternals;
    std::vector<std::pair<jerry_value_t, jerry_value_t>> m_global_symbols;
    std::vector<JerryValue*> m_micro_tasks;
    std::unordered_map<uint16_t*, std::u16string*> m_utf16strs;

    JerryObjectTemplate* m_hidden_object_template;

    JerryPolyfill* m_fn_map_set;
    JerryPolyfill* m_fn_set_add;
    JerryPolyfill* m_fn_object_assign;
    JerryPolyfill* m_fn_conversion_failer;
    JerryPolyfill* m_fn_get_own_prop;
    JerryPolyfill* m_fn_get_own_names;
    JerryPolyfill* m_fn_set_integrity;

    JerryValue* m_magic_string_stack;
    JerryValue* m_call_site_prototype;

    JerryHandleScope* m_last_handle_scope;
    JerryValue** m_last_return_value;
    v8::TryCatch* m_last_try_catch;
    bool m_has_current_error;
    jerry_value_t m_current_error;
    JerryValue* m_global_error;
    size_t m_try_depth;
    v8::PromiseHook m_promise_hook;
    v8::PromiseRejectCallback m_promise_reject_callback;

    v8::FatalErrorCallback m_fatalErrorCallback;
    v8::Isolate::AbortOnUncaughtExceptionCallback m_abortOnUncaughtExceptionCallback;
    v8::MessageCallback m_messageCallback;
    v8::PrepareStackTraceCallback m_prepareStackTraceCallback;
    v8::HostImportModuleDynamicallyCallback m_importModuleDynamicallyCallback;

    bool m_terminated;
    bool m_autorun_tasks;
};

#endif /* V8JERRY_ISOLATE_HPP */
