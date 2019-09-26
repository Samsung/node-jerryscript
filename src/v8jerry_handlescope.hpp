#ifndef V8JERRY_HANDLESCOPE_HPP
#define V8JERRY_HANDLESCOPE_HPP

#include <vector>

class JerryHandle;

enum JerryHandleScopeType : int {
    Normal = 0, // Normal and Escapable scopes are treated the as same atm.
    //Escapable,
    Sealed,
};

class JerryHandleScope {
public:
    JerryHandleScope(JerryHandleScopeType type, void* handle_scope)
        : m_type(type)
        , m_v8handle_scope(handle_scope)
    {
    }

    ~JerryHandleScope();

    void AddHandle(JerryHandle* jvalue);
    void RemoveHandle(JerryHandle* jvalue);

    void* V8HandleScope(void) { return m_v8handle_scope; }

    void Seal(void) { m_type = Sealed; }

private:
    JerryHandleScopeType m_type;
    void* m_v8handle_scope;
    std::vector<JerryHandle*> m_handles;
};

#endif /* V8JERRY_HANDLESCOPE_HPP */
