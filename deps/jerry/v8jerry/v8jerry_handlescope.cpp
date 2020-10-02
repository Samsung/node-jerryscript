#include <algorithm>
#include <cstdio>

#include "v8jerry_handlescope.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_value.hpp"

JerryHandleScope::~JerryHandleScope(void) {
    for (std::vector<JerryHandle*>::reverse_iterator it = m_handles.rbegin(); it != m_handles.rend(); it++) {
        delete reinterpret_cast<JerryValue*>(*it);
    }
}

void JerryHandleScope::AddHandle(JerryHandle* jvalue) {
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

bool JerryHandleScope::RemoveHandle(JerryHandle* jvalue) {
    std::vector<JerryHandle*>::iterator it = std::find(m_handles.begin(), m_handles.end(), jvalue);

    if (it == m_handles.end()) {
        return false;
    }

    m_handles.erase(it);
    return true;
}
