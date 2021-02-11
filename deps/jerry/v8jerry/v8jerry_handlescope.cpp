#include <algorithm>
#include <cstdio>

#include "v8jerry_handlescope.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_value.hpp"

void JerryHandleScope::FreeHandles(JerryValue* LastReturnValue) {
    std::vector<JerryValue*>::iterator end = m_handles.end();
    for (std::vector<JerryValue*>::iterator it = m_handles.begin(); it != end; it++) {
        JerryValue* value = *it;

        if (value == LastReturnValue) {
            PrevHandleScope()->AddHandle(value);
        } else {
            delete value;
        }
    }
}

void JerryHandleScope::AddHandle(JerryValue* jvalue) {
    if (m_type == Sealed) {
        fprintf(stderr, "Invalid usage of handles: Using SealHandleScope for variables\n");
        JerryIsolate::GetCurrent()->ReportFatalError("", "Trying to add handle to SealHandleScope");
        return;
    }

    if (jvalue->type() != JerryHandle::LocalValue) {
        fprintf(stderr, "JerryHandleScope::AddHandle: Unexpected handle type (%d)\n", jvalue->type());
        abort();
    }

    m_handles.push_back(jvalue);
}

bool JerryHandleScope::RemoveHandle(JerryValue* jvalue) {
    std::vector<JerryValue*>::iterator it = std::find(m_handles.begin(), m_handles.end(), jvalue);

    if (it == m_handles.end()) {
        return false;
    }

    m_handles.erase(it);
    return true;
}
