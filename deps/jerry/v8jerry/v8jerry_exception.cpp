#include "v8jerry_exception.hpp"
#include "v8jerry_isolate.hpp"

struct MessageData {
    jerry_value_t source_string;
    jerry_value_t resource_name;
    int line;
    int column;
    int relative_line;
};

void MessageDataFree(void* native_p, jerry_object_native_info_t* info_p) {
    (void) info_p;

    MessageData* data = reinterpret_cast<MessageData*>(native_p);

    jerry_release_value(data->resource_name);
    jerry_release_value(data->source_string);
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

    const jerry_backtrace_location_t* location = jerry_backtrace_get_location(frame_p);

    if (location != NULL) {
        data->resource_name = jerry_acquire_value(location->resource_name);
        data->line = static_cast<int>(location->line);
        data->column = static_cast<int>(location->column - 1);

        const jerry_value_t *function = jerry_backtrace_get_function(frame_p);
        if (function != NULL) {
            jerry_source_info_t *source_info = jerry_get_source_info(*function);

            if (source_info != NULL && (source_info->enabled_fields & JERRY_SOURCE_INFO_HAS_SOURCE_CODE)) {
                jerry_release_value(data->source_string);
                data->source_string = jerry_acquire_value(source_info->source_code);
            }
            jerry_free_source_info(source_info);

            data->relative_line = data->line - 1;
        }
    }

    return false;
}

void JerryThrowCallback(const jerry_value_t error_value, void* user_p) {
    jerry_value_t message_value = jerry_create_object();

    MessageData* data = new MessageData;
    data->source_string = jerry_create_string((const jerry_char_t *) "");
    data->line = 0;
    data->column = 0;
    data->relative_line = 0;

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

void JerryThrowSyntaxError(jerry_value_t error_value, jerry_value_t source_string, jerry_parse_options_t* parse_options, void* user_p) {
    int line = 1;
    int column = 1;

    jerry_value_t name_value = jerry_create_string(reinterpret_cast<const jerry_char_t*>("message"));
    jerry_value_t error_object = jerry_get_value_from_error(error_value, false);
    jerry_value_t message_value = jerry_get_property(error_object, name_value);
    jerry_release_value(error_object);
    jerry_release_value(name_value);

    if (jerry_value_is_string(message_value)) {
        jerry_size_t size = jerry_get_string_size(message_value);
        jerry_char_t* message = reinterpret_cast<jerry_char_t*>(malloc(size));
        jerry_string_to_char_buffer(message_value, message, size);

        jerry_char_t* ptr = message + size;
        if (ptr > message && ptr[-1] == ']') {
            ptr--;
        } else {
            ptr = message;
        }

        if (ptr > message && ptr[-1] >= '0' && ptr[-1] <= '9') {
            do {
                ptr--;
            } while (ptr > message && ptr[-1] >= '0' && ptr[-1] <= '9');
        } else {
            ptr = message;
        }

        if (ptr > message && ptr[-1] == ':') {
            ptr--;
        } else {
            ptr = message;
        }

        if (ptr > message && ptr[-1] >= '0' && ptr[-1] <= '9') {
            do {
                ptr--;
            } while (ptr > message && ptr[-1] >= '0' && ptr[-1] <= '9');
        } else {
            ptr = message;
        }

        if (ptr > message && ptr[-1] != ':') {
            ptr = message;
        }

        if (ptr > message) {
            line = 0;
            column = 0;

            do {
                line = line * 10 + (*ptr++) - '0';
            } while (*ptr != ':');

            ptr++;

            do {
                column = column * 10 + (*ptr++) - '0';
            } while (*ptr != ']');
        }

        free(message);
    }
    jerry_release_value(message_value);

    message_value = jerry_create_object();

    MessageData* data = new MessageData;

    data->source_string = jerry_acquire_value(source_string);
    data->resource_name = jerry_acquire_value(parse_options->resource_name);
    data->line = line;
    data->column = column - 1;
    data->relative_line = line - static_cast<int>(parse_options->start_line);

    jerry_set_object_native_pointer(message_value, reinterpret_cast<void*>(data), &MessageDataTypeInfo);

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

int JerryMessageGetColumnStartNumber(jerry_value_t message_value) {
    MessageData* data;

    if (jerry_get_object_native_pointer(message_value, reinterpret_cast<void**>(&data), &MessageDataTypeInfo)) {
        return data->column;
    }

    return 0;
}

int JerryMessageGetColumnEndNumber(jerry_value_t message_value) {
    MessageData* data;

    if (jerry_get_object_native_pointer(message_value, reinterpret_cast<void**>(&data), &MessageDataTypeInfo)) {
        return data->column + 1;
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

jerry_value_t JerryMessageGetSourceLine(jerry_value_t message_value)
{
    MessageData* data;

    if (!jerry_get_object_native_pointer(message_value, reinterpret_cast<void**>(&data), &MessageDataTypeInfo)) {
        return jerry_create_string((const jerry_char_t *) "");
    }

    jerry_size_t size = jerry_get_string_size (data->source_string);
    jerry_char_t* buffer = reinterpret_cast<jerry_char_t*>(malloc(size));

    jerry_string_to_char_buffer(data->source_string, buffer, size);

    jerry_char_t* buffer_end = buffer + size;
    jerry_char_t* start = buffer;
    jerry_char_t* end = buffer;
    int relative_line = data->relative_line;

    while (end < buffer_end) {
        size_t new_line_length = 1;

        switch (*end) {
            case '\r':
                if (end + 2 <= buffer_end && end[1] == '\n') {
                    new_line_length = 2;
                }
                break;
            case '\n':
                break;
            case 0xe2:
                if (end + 3 <= buffer_end && end[1] == 0x80 && (end[2] | 0x1) == 0xa9) {
                    new_line_length = 3;
                    break;
                }
                /* FALLTHRU */
            default:
                end++;
                continue;
        }

        if (relative_line == 0) {
            break;
        }

        end += new_line_length;

        if (--relative_line == 0) {
            start = end;
        }
    }

    if (relative_line != 0) {
        /* Should never happen */
        return jerry_create_string((const jerry_char_t *) "");
    }

    jerry_value_t result = jerry_create_string_sz(start, static_cast<jerry_size_t>(end - start));
    free(buffer);

    return result;
}
