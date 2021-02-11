#ifndef V8JERRY_HANDLESCOPE_HPP
#define V8JERRY_HANDLESCOPE_HPP

#include <vector>

class JerryValue;

enum JerryHandleScopeType : int {
    Normal = 0, // Normal and Escapable scopes are treated the as same atm.
    //Escapable,
    Sealed,
};

class JerryHandleScope {
public:
    JerryHandleScope(JerryHandleScopeType type, JerryHandleScope* prev_handle_scope)
        : m_type(type)
        , m_prev_handle_scope(prev_handle_scope)
    {
    }

    void AddHandle(JerryValue* jvalue);
    bool RemoveHandle(JerryValue* jvalue);
    void FreeHandles(JerryValue* LastReturnValue);

    JerryHandleScope* PrevHandleScope(void) { return m_prev_handle_scope; }

    void Seal(void) { m_type = Sealed; }

private:
    JerryHandleScopeType m_type;
    JerryHandleScope* m_prev_handle_scope;
    std::vector<JerryValue*> m_handles;
};

#endif /* V8JERRY_HANDLESCOPE_HPP */
