#include "v8jerry_exception.hpp"
#include "v8jerry_isolate.hpp"

struct MessageData {
    jerry_value_t resource_name;
    int line;
};

void MessageDataFree(void* native_p, jerry_object_native_info_t* info_p) {
    (void) info_p;

    MessageData* data = reinterpret_cast<MessageData*>(native_p);

    jerry_release_value(data->resource_name);
    delete data;
}

static jerry_object_native_info_t MessageDataTypeInfo = {
    .free_cb = MessageDataFree,
    .number_of_references = 0,
    .offset_of_references = 0,
};

static bool BacktraceCallback(jerry_backtrace_frame_t* frame_p, void* user_p) {
    if (jerry_backtrace_get_frame_type(frame_p) != JERRY_BACKTRACE_FRAME_JS) {
        return true;
    }

    MessageData* data = reinterpret_cast<MessageData*>(user_p);

    const jerry_backtrace_location_t* location = jerry_backtrace_get_location (frame_p);

    if (location != NULL) {
        data->resource_name = jerry_acquire_value(location->resource_name);
        data->line = static_cast<int>(location->line);
    }

    return false;
}

void JerryThrowCallback(const jerry_value_t error_value, void* user_p) {
    jerry_value_t message_value = jerry_create_object();

    MessageData* data = new MessageData;
    data->line = 0;

    jerry_backtrace_capture(BacktraceCallback, reinterpret_cast<void*>(data));
    jerry_set_object_native_pointer(message_value, reinterpret_cast<void*>(data), &MessageDataTypeInfo);

    if (data->line == 0) {
        data->resource_name = jerry_create_string((const jerry_char_t *) "");
        data->line = 1;
    }

    JerryIsolate *isolate = reinterpret_cast<JerryIsolate*>(user_p);
    isolate->SetMessage(message_value);
    jerry_release_value(message_value);
}

int JerryMessageGetLineNumber(jerry_value_t message_value) {
    MessageData* data;

    if (jerry_get_object_native_pointer(message_value, reinterpret_cast<void**>(&data), &MessageDataTypeInfo)) {
        return data->line;
    }

    return 0;
}

jerry_value_t JerryMessageGetScriptResourceName(jerry_value_t message_value) {
    MessageData* data;

    if (jerry_get_object_native_pointer(message_value, reinterpret_cast<void**>(&data), &MessageDataTypeInfo)) {
        return jerry_acquire_value(data->resource_name);
    }

    return jerry_create_string((const jerry_char_t *) "");
}
