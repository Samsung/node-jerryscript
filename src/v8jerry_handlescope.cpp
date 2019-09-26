#include <algorithm>
#include <cstdio>

#include "v8jerry_context.hpp"
#include "v8jerry_handlescope.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_value.hpp"

JerryHandleScope::~JerryHandleScope(void) {
    for (std::vector<JerryHandle*>::reverse_iterator it = m_handles.rbegin();
        it != m_handles.rend();
        it++) {
        JerryHandle* jhandle = *it;

        if (JerryIsolate::GetCurrent()->HasEternal(reinterpret_cast<JerryValue*>(jhandle))) {
            continue;
        }

        switch (jhandle->type()) {
            case JerryHandle::Value: delete reinterpret_cast<JerryValue*>(jhandle); break;
            case JerryHandle::Context: delete reinterpret_cast<JerryContext*>(jhandle); break;
            // FunctionTemplate and ObjectTemplates are now Isolate level items.
            default: fprintf(stderr, "~JerryHandleScope::Unsupported handle type (%d)\n", jhandle->type()); break;
        }
    }
}

void JerryHandleScope::AddHandle(JerryHandle* jvalue) {
    if (m_type == Sealed) {
        fprintf(stderr, "Invalid usage of handles: Using SealHandleScope for variables\n");
        JerryIsolate::GetCurrent()->ReportFatalError("", "Trying to add handle to SealHandleScope");
        return;
    }

    m_handles.push_back(jvalue);
}

void JerryHandleScope::RemoveHandle(JerryHandle* jvalue) {
    // TODO: check if it really exists
    m_handles.erase(std::find(m_handles.begin(), m_handles.end(), jvalue));
}
