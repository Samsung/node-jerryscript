#include "v8jerry_serialize.hpp"
#include "v8jerry_isolate.hpp"
#include "v8jerry_utils.hpp"

#include <cassert>
#include <cmath>

namespace JerrySerialize {

// Serializer bases on v8 version
static const uint32_t kLatestVersion = 13;
enum class SerializationTag : uint8_t {
    // version:uint32_t (if at beginning of data, sets version > 0)
    kVersion = 0xFF,
    // ignore
    kPadding = '\0',
    // refTableSize:uint32_t (previously used for sanity checks; safe to ignore)
    kVerifyObjectCount = '?',
    // Oddballs (no data).
    kTheHole = '-',
    kUndefined = '_',
    kNull = '0',
    kTrue = 'T',
    kFalse = 'F',
    // Number represented as 32-bit integer, ZigZag-encoded
    // (like sint32 in protobuf)
    kInt32 = 'I',
    // Number represented as 32-bit unsigned integer, varint-encoded
    // (like uint32 in protobuf)
    kUint32 = 'U',
    // Number represented as a 64-bit double.
    // Host byte order is used (N.B. this makes the format non-portable).
    kDouble = 'N',
    // BigInt. Bitfield:uint32_t, then raw digits storage.
    kBigInt = 'Z',
    // byteLength:uint32_t, then raw data
    kUtf8String = 'S',
    kOneByteString = '"',
    kTwoByteString = 'c',
    // Reference to a serialized object. objectID:uint32_t
    kObjectReference = '^',
    // Beginning of a JS object.
    kBeginJSObject = 'o',
    // End of a JS object. numProperties:uint32_t
    kEndJSObject = '{',
    // Beginning of a sparse JS array. length:uint32_t
    // Elements and properties are written as key/value pairs, like objects.
    kBeginSparseJSArray = 'a',
    // End of a sparse JS array. numProperties:uint32_t length:uint32_t
    kEndSparseJSArray = '@',
    // Beginning of a dense JS array. length:uint32_t
    // |length| elements, followed by properties as key/value pairs
    kBeginDenseJSArray = 'A',
    // End of a dense JS array. numProperties:uint32_t length:uint32_t
    kEndDenseJSArray = '$',
    // Date. millisSinceEpoch:double
    kDate = 'D',
    // Boolean object. No data.
    kTrueObject = 'y',
    kFalseObject = 'x',
    // Number object. value:double
    kNumberObject = 'n',
    // BigInt object. Bitfield:uint32_t, then raw digits storage.
    kBigIntObject = 'z',
    // String object, UTF-8 encoding. byteLength:uint32_t, then raw data.
    kStringObject = 's',
    // Regular expression, UTF-8 encoding. byteLength:uint32_t, raw data,
    // flags:uint32_t.
    kRegExp = 'R',
    // Beginning of a JS map.
    kBeginJSMap = ';',
    // End of a JS map. length:uint32_t.
    kEndJSMap = ':',
    // Beginning of a JS set.
    kBeginJSSet = '\'',
    // End of a JS set. length:uint32_t.
    kEndJSSet = ',',
    // Array buffer. byteLength:uint32_t, then raw data.
    kArrayBuffer = 'B',
    // Array buffer (transferred). transferID:uint32_t
    kArrayBufferTransfer = 't',
    // View into an array buffer.
    // subtag:ArrayBufferViewTag, byteOffset:uint32_t, byteLength:uint32_t
    // For typed arrays, byteOffset and byteLength must be divisible by the size
    // of the element.
    // Note: kArrayBufferView is special, and should have an ArrayBuffer (or an
    // ObjectReference to one) serialized just before it. This is a quirk
    // arising
    // from the previous stack-based implementation.
    kArrayBufferView = 'V',
    // Shared array buffer. transferID:uint32_t
    kSharedArrayBuffer = 'u',
    // Compiled WebAssembly module. encodingType:(one-byte tag).
    // If encodingType == 'y' (raw bytes):
    //  wasmWireByteLength:uint32_t, then raw data
    //  compiledDataLength:uint32_t, then raw data
    kWasmModule = 'W',
    // A wasm module object transfer. next value is its index.
    kWasmModuleTransfer = 'w',
    // The delegate is responsible for processing all following data.
    // This "escapes" to whatever wire format the delegate chooses.
    kHostObject = '\\',
    // A transferred WebAssembly.Memory object. maximumPages:int32_t, then by
    // SharedArrayBuffer tag and its data.
    kWasmMemoryTransfer = 'm',
    // A list of (subtag: ErrorTag, [subtag dependent data]). See ErrorTag for
    // details.
    kError = 'r',

    // The following tags are reserved because they were in use in Chromium
    // before
    kLegacyReservedMessagePort = 'M',
    kLegacyReservedBlob = 'b',
    kLegacyReservedBlobIndex = 'i',
    kLegacyReservedFile = 'f',
    kLegacyReservedFileIndex = 'e',
    kLegacyReservedDOMFileSystem = 'd',
    kLegacyReservedFileList = 'l',
    kLegacyReservedFileListIndex = 'L',
    kLegacyReservedImageData = '#',
    kLegacyReservedImageBitmap = 'g',
    kLegacyReservedImageBitmapTransfer = 'G',
    kLegacyReservedOffscreenCanvas = 'H',
    kLegacyReservedCryptoKey = 'K',
    kLegacyReservedRTCCertificate = 'k',
};

// Sub-tags only meaningful for error serialization.
enum class ErrorTag : uint8_t {
    // The error is a EvalError. No accompanying data.
    kEvalErrorPrototype = 'E',
    // The error is a RangeError. No accompanying data.
    kRangeErrorPrototype = 'R',
    // The error is a ReferenceError. No accompanying data.
    kReferenceErrorPrototype = 'F',
    // The error is a SyntaxError. No accompanying data.
    kSyntaxErrorPrototype = 'S',
    // The error is a TypeError. No accompanying data.
    kTypeErrorPrototype = 'T',
    // The error is a URIError. No accompanying data.
    kUriErrorPrototype = 'U',
    // Followed by message: string.
    kMessage = 'm',
    // Followed by stack: string.
    kStack = 's',
    // The end of this error information.
    kEnd = '.',
};

ValueSerializer::ValueSerializer(i::Isolate* isolate,
                                 v8::ValueSerializer::Delegate* delegate)
    : isolate_(isolate), delegate_(delegate) {}

void ValueSerializer::WriteHeader() {
    WriteTag(SerializationTag::kVersion);
    WriteVarint(kLatestVersion);
}

bool ValueSerializer::WriteValue(JerryValue* value) {
    return WriteValueInternal(value->value());
}

void ValueSerializer::WriteUint32(uint32_t value) {
    WriteVarint<uint32_t>(value);
}

void ValueSerializer::WriteUint64(uint64_t value) {
    WriteVarint<uint64_t>(value);
}

void ValueSerializer::WriteDouble(double value) {
    WriteRawBytes(&value, sizeof(value));
}

void ValueSerializer::WriteRawBytes(const void* source, size_t length) {
    buffer_.Append(source, length);
}

std::pair<uint8_t*, size_t> ValueSerializer::Release() {
    size_t size = buffer_.Size();
    uint8_t* b = buffer_.Release();
    auto result = std::make_pair(b, size);
    return result;
}

void ValueSerializer::SetTreatArrayBufferViewsAsHostObjects(bool mode) {
    treat_array_buffer_views_as_host_objects_ = mode;
}

bool ValueSerializer::WriteValueInternal(jerry_value_t value) {
    if (jerry_value_is_undefined(value)) {
        WriteTag(SerializationTag::kUndefined);
    } else if (jerry_value_is_null(value)) {
        WriteTag(SerializationTag::kNull);
    } else if (jerry_value_is_boolean(value)) {
        if (jerry_get_boolean_value(value)) {
            WriteTag(SerializationTag::kTrue);
        } else {
            WriteTag(SerializationTag::kFalse);
        }
    } else if (jerry_value_is_number(value)) {
        if (jerry_get_number_value(value) ==
            (uint32_t)jerry_get_number_value(value)) {
            WriteTag(SerializationTag::kUint32);
            WriteVarint<uint32_t>(jerry_get_number_value(value));
        } else if (jerry_get_number_value(value) ==
                   (int32_t)jerry_get_number_value(value)) {
            WriteTag(SerializationTag::kInt32);
            WriteZigZag<int32_t>(jerry_get_number_value(value));
        } else {
            WriteTag(SerializationTag::kDouble);
            WriteDouble(jerry_get_number_value(value));
        }
    } else if (jerry_value_is_string(value)) {
        WriteJerryString(value);
    } else if (jerry_value_is_arraybuffer(value)) {
        return WriteJerryArrayBuffer(value);
    } else if (jerry_value_is_array(value)) {
        return WriteJerryArray(value);
    } else if (jerry_value_is_error(value) ||
               jerry_get_error_type(value) != JERRY_ERROR_NONE) {
        // TODO: Errors are not properly recognized.
        // jerry_value_is_error(value) returns false for error objects, but
        // jerry_get_error_type(value) returns valid error type
        WriteJerryError(value);
    } else if (jerry_value_is_symbol(value)) {
        reinterpret_cast<JerryIsolate*>(isolate_)->SetError(jerry_create_error(
            JERRY_ERROR_COMMON,
            (const jerry_char_t*)"Symbol could not be serialized"));
        return false;
    } else if (jerry_value_is_function(value)) {
        reinterpret_cast<JerryIsolate*>(isolate_)->SetError(jerry_create_error(
            JERRY_ERROR_COMMON,
            (const jerry_char_t*)"Function could not be serialized"));
        return false;
    } else if (jerry_value_is_object(value)) {
        WriteJerryObject(value);
    } else {
        reinterpret_cast<JerryIsolate*>(isolate_)->SetError(jerry_create_error(
            JERRY_ERROR_COMMON, (const jerry_char_t*)"Serialization error"));
    }
    return true;
}

void ValueSerializer::WriteTag(SerializationTag tag) {
    uint8_t raw_tag = static_cast<uint8_t>(tag);
    WriteRawBytes(&raw_tag, sizeof(raw_tag));
}

template <typename T>
void ValueSerializer::WriteVarint(T value) {
    // Writes an unsigned integer as a base-128 varint.
    // The number is written, 7 bits at a time, from the least significant to
    // the most significant 7 bits. Each byte, except the last, has the MSB set.
    // See also https://developers.google.com/protocol-buffers/docs/encoding
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                  "Only unsigned integer types can be written as varints.");
    uint8_t stack_buffer[sizeof(T) * 8 / 7 + 1];
    uint8_t* next_byte = &stack_buffer[0];
    do {
        *next_byte = (value & 0x7F) | 0x80;
        next_byte++;
        value >>= 7;
    } while (value);
    *(next_byte - 1) &= 0x7F;
    WriteRawBytes(stack_buffer, next_byte - stack_buffer);
}

template <typename T>
void ValueSerializer::WriteZigZag(T value) {
    // Writes a signed integer as a varint using ZigZag encoding (i.e. 0 is
    // encoded as 0, -1 as 1, 1 as 2, -2 as 3, and so on).
    // See also https://developers.google.com/protocol-buffers/docs/encoding
    // Note that this implementation relies on the right shift being arithmetic.
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value,
                  "Only signed integer types can be written as zigzag.");
    using UnsignedT = typename std::make_unsigned<T>::type;
    WriteVarint((static_cast<UnsignedT>(value) << 1) ^
                (value >> (8 * sizeof(T) - 1)));
}

bool ValueSerializer::WriteJerryArray(jerry_value_t value) {
    size_t length = jerry_get_array_length(value);
    uint32_t properties_written = 0;
    WriteTag(SerializationTag::kBeginDenseJSArray);
    WriteVarint<uint32_t>(length);
    jerry_value_t keys = jerry_get_object_keys(value);
    uint32_t len = jerry_get_array_length(keys);
    for (uint32_t i = 0; i < len; i++) {
        jerry_value_t prop_key = jerry_get_property_by_index(keys, i);
        jerry_value_t prop_value = jerry_get_property(value, prop_key);
        jerry_value_t prop_key_num = jerry_value_to_number(prop_key);
        bool success =
            WriteValueInternal(prop_key_num) && WriteValueInternal(prop_value);
        jerry_release_value(prop_value);
        jerry_release_value(prop_key_num);
        jerry_release_value(prop_key);
        if (success == false) {
            return false;
        }
        properties_written++;
    }
    jerry_release_value(keys);
    WriteTag(SerializationTag::kEndDenseJSArray);
    WriteVarint<uint32_t>(properties_written);
    WriteVarint<uint32_t>(length);
    return true;
}

bool ValueSerializer::WriteJerryArrayBuffer(jerry_value_t value) {
    WriteTag(SerializationTag::kArrayBuffer);
    size_t length = jerry_get_arraybuffer_byte_length(value);
    uint8_t* bytes = jerry_get_arraybuffer_pointer(value);
    WriteVarint<uint32_t>(length);
    WriteRawBytes(bytes, length);
    return true;
}

void ValueSerializer::WriteJerryString(jerry_value_t value) {
    WriteTag(SerializationTag::kOneByteString);
    size_t len = jerry_get_string_length(value);
    jerry_char_t* buf = new jerry_char_t[len + 1];
    jerry_size_t bytes = jerry_string_to_char_buffer(value, buf, len + 1);
    WriteVarint<uint32_t>(bytes);
    WriteRawBytes(buf, bytes);
    delete[] buf;
    return;
}

void ValueSerializer::WriteJerryError(jerry_value_t value) {
    WriteTag(SerializationTag::kError);
    jerry_error_t error_type = jerry_get_error_type(value);

    if (error_type == JERRY_ERROR_EVAL) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kEvalErrorPrototype));
    } else if (error_type == JERRY_ERROR_RANGE) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kRangeErrorPrototype));
    } else if (error_type == JERRY_ERROR_REFERENCE) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kReferenceErrorPrototype));
    } else if (error_type == JERRY_ERROR_SYNTAX) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kSyntaxErrorPrototype));
    } else if (error_type == JERRY_ERROR_TYPE) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kTypeErrorPrototype));
    } else if (error_type == JERRY_ERROR_URI) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kUriErrorPrototype));
    } else {
        // JERRY_ERROR_COMMON
    }

    // message
    jerry_value_t prop_name =
        jerry_create_string((const jerry_char_t*)"message");
    if (jerry_has_property(value, prop_name)) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kMessage));
        jerry_value_t prop_value = jerry_get_property(value, prop_name);
        WriteValueInternal(prop_value);
        jerry_release_value(prop_value);
    }
    jerry_release_value(prop_name);

    // stack
    prop_name = jerry_create_string((const jerry_char_t*)"stack");
    if (jerry_has_property(value, prop_name)) {
        WriteVarint(static_cast<uint8_t>(ErrorTag::kStack));
        jerry_value_t prop_value = jerry_get_property(value, prop_name);
        WriteValueInternal(prop_value);
        jerry_release_value(prop_value);
    }
    jerry_release_value(prop_name);

    WriteVarint(static_cast<uint8_t>(ErrorTag::kEnd));
}

bool ValueSerializer::WriteJerryObject(jerry_value_t value) {
    uint32_t properties_written = 0;
    WriteTag(SerializationTag::kBeginJSObject);
    jerry_value_t keys =
        jerry_object_get_property_names(value, JERRY_PROPERTY_FILTER_ALL);
    uint32_t len = jerry_get_array_length(keys);
    for (uint32_t i = 0; i < len; i++) {
        jerry_value_t prop_key = jerry_get_property_by_index(keys, i);
        jerry_value_t prop_value = jerry_get_property(value, prop_key);
        bool success =
            WriteValueInternal(prop_key) && WriteValueInternal(prop_value);
        jerry_release_value(prop_value);
        jerry_release_value(prop_key);
        if (!success) {
            return false;
        }
        properties_written++;
    }
    jerry_release_value(keys);
    WriteTag(SerializationTag::kEndJSObject);
    WriteVarint<uint32_t>(properties_written);
    return true;
}

ValueDeserializer::ValueDeserializer(i::Isolate* isolate,
                                     const uint8_t* data,
                                     const size_t size,
                                     v8::ValueDeserializer::Delegate* delegate)
    : buffer_(data),
      position_(0),
      size_(size),
      isolate_(isolate),
      delegate_(delegate) {}

bool ValueDeserializer::ReadHeader() {
    if (position_ < size_) {
        if (buffer_[position_] ==
            static_cast<uint8_t>(SerializationTag::kVersion)) {
            position_++;
            uint32_t version_ = 0;
            if (ReadVarint<uint32_t>(&version_)) {
                if (version_ <= kLatestVersion) {
                    return true;
                }
            }
        }
    }
    return false;
}

JerryValue* ValueDeserializer::ReadValue() {
    jerry_value_t result = ReadValueInternal();
    return new JerryValue(result);
}

bool ValueDeserializer::ReadDouble(double* value) {
    if (position_ > size_ - sizeof(double)) {
        return false;
    }
    *value = 0;
    memcpy(value, &buffer_[position_], sizeof(double));
    position_ += sizeof(double);
    if (std::isnan(*value)) {
        return false;
    }
    return true;
}

bool ValueDeserializer::ReadUint32(uint32_t* value) {
    return ReadVarint<uint32_t>(value);
}
bool ValueDeserializer::ReadUint64(uint64_t* value) {
    return ReadVarint<uint64_t>(value);
}

bool ValueDeserializer::ReadRawBytes(size_t length, const void** data) {
    if (length == 0 || length > static_cast<size_t>(size_ - position_)) {
        return false;
    }
    *data = &buffer_[position_];
    position_ += length;
    return true;
}

jerry_value_t ValueDeserializer::ReadValueInternal() {
    SerializationTag tag;
    if (!ReadTag(&tag)) {
        return jerry_create_error(JERRY_ERROR_COMMON,
                                  (const jerry_char_t*)"Deserialization error");
    }
    switch (tag) {
        case SerializationTag::kUndefined:
            return jerry_create_undefined();
        case SerializationTag::kNull:
            return jerry_create_null();

        case SerializationTag::kTrue:
            return jerry_create_boolean(true);

        case SerializationTag::kFalse:
            return jerry_create_boolean(false);

        case SerializationTag::kUint32: {
            uint32_t v = 0;
            ReadVarint<uint32_t>(&v);
            return jerry_create_number(v);
        }
        case SerializationTag::kInt32: {
            int32_t v = 0;
            ReadZigZag<int32_t>(&v);
            return jerry_create_number(v);
        }
        case SerializationTag::kDouble: {
            double v = 0;
            ReadDouble(&v);
            return jerry_create_number(v);
            break;
        }
        case SerializationTag::kOneByteString: {
            uint32_t length = 0;
            const void* str = nullptr;
            if (ReadVarint<uint32_t>(&length) && ReadRawBytes(length, &str)) {
                return jerry_create_string_sz((const jerry_char_t*)str, length);
            } else {
                return jerry_create_string_sz((const jerry_char_t*)str, 0);
            }
        }
        case SerializationTag::kArrayBuffer: {
            return ReadJerryArrayBuffer();
        }
        case SerializationTag::kBeginDenseJSArray: {
            return ReadJerryArray();
        }
        case SerializationTag::kBeginJSObject: {
            return ReadJerryObject();
        }
        case SerializationTag::kError:
            return ReadJerryError();
        default:
            break;
    }

    return jerry_create_error(JERRY_ERROR_COMMON,
                              (const jerry_char_t*)"Deserialization error");
}

bool ValueDeserializer::ReadTag(SerializationTag* tag) {
    do {
        if (position_ >= size_) {
            return false;
        }
        *tag = static_cast<SerializationTag>(buffer_[position_]);
        position_++;
    } while (*tag == SerializationTag::kPadding);
    return true;
}

bool ValueDeserializer::ReadErrorTag(ErrorTag* tag) {
    if (position_ >= size_) {
        return false;
    }
    *tag = static_cast<ErrorTag>(buffer_[position_]);
    position_++;
    return true;
}

bool ValueDeserializer::CheckTag(SerializationTag check) {
    SerializationTag tag;
    size_t p = position_;
    do {
        if (p >= size_) {
            return false;
        }
        tag = static_cast<SerializationTag>(buffer_[p]);
        p++;
    } while (tag == SerializationTag::kPadding);

    return tag == check;
}

template <typename T>
bool ValueDeserializer::ReadVarint(T* value) {
    // Reads an unsigned integer as a base-128 varint.
    // The number is written, 7 bits at a time, from the least significant to
    // the most significant 7 bits. Each byte, except the last, has the MSB set.
    // If the varint is larger than T, any more significant bits are discarded.
    // See also https://developers.google.com/protocol-buffers/docs/encoding
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                  "Only unsigned integer types can be read as varints.");
    *value = 0;
    unsigned shift = 0;
    bool has_another_byte;
    do {
        if (position_ >= size_) return false;
        uint8_t byte = buffer_[position_];
        if (shift < sizeof(T) * 8) {
            *value |= static_cast<T>(byte & 0x7F) << shift;
            shift += 7;
        }
        has_another_byte = byte & 0x80;
        position_++;
    } while (has_another_byte);
    return true;
}

template <typename T>
bool ValueDeserializer::ReadZigZag(T* value) {
    // Writes a signed integer as a varint using ZigZag encoding (i.e. 0 is
    // encoded as 0, -1 as 1, 1 as 2, -2 as 3, and so on).
    // See also https://developers.google.com/protocol-buffers/docs/encoding
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value,
                  "Only signed integer types can be read as zigzag.");
    using UnsignedT = typename std::make_unsigned<T>::type;
    UnsignedT unsigned_value;
    if (!ReadVarint<UnsignedT>(&unsigned_value)) {
        return false;
    }
    *value = static_cast<T>((unsigned_value >> 1) ^
                            -static_cast<T>(unsigned_value & 1));
    return true;
}

bool ValueDeserializer::ReadOneByteString(JerryString* value) {
    return true;
}

jerry_value_t ValueDeserializer::ReadJerryArray() {
    uint32_t length;
    if (!ReadVarint<uint32_t>(&length)) {
        return jerry_create_undefined();
    }
    jerry_value_t result = jerry_create_array(length);
    size_t properties_read = 0;
    while (!CheckTag(SerializationTag::kEndDenseJSArray)) {
        jerry_value_t prop_index = ReadValueInternal();
        if (!jerry_value_is_undefined(prop_index)) {
            jerry_value_t prop_value = ReadValueInternal();
            jerry_value_t res = jerry_set_property_by_index(
                result, jerry_get_number_value(prop_index), prop_value);
            properties_read++;
            jerry_release_value(res);
            jerry_release_value(prop_value);
            jerry_release_value(prop_index);
        }
    }
    SerializationTag tag;
    ReadTag(&tag);
    if (tag == SerializationTag::kEndDenseJSArray) {
        uint32_t properties_written, length_written;
        if (ReadVarint<uint32_t>(&properties_written) &&
            ReadVarint<uint32_t>(&length_written)) {
            if (properties_read == properties_written &&
                length == length_written) {
                return result;
            }
        }
    }
    jerry_release_value(result);
    return jerry_create_undefined();
}

jerry_value_t ValueDeserializer::ReadJerryArrayBuffer() {
    uint32_t length;
    if (!ReadVarint<uint32_t>(&length)) {
        return jerry_create_undefined();
    }
    const void* bytes = nullptr;
    ReadRawBytes(length, &bytes);
    jerry_value_t result = jerry_create_arraybuffer(length);
    jerry_arraybuffer_write(result, 0, (const uint8_t*)bytes, length);
    return result;
}

jerry_value_t ValueDeserializer::ReadJerryError() {
    ErrorTag tag;
    jerry_error_t error_type = JERRY_ERROR_COMMON;
    jerry_value_t message = jerry_create_undefined();
    jerry_value_t stack = jerry_create_undefined();
    while (true) {
        if (!ReadErrorTag(&tag)) {
            return jerry_create_error(
                JERRY_ERROR_COMMON,
                (const jerry_char_t*)"Deserialization error");
        }
        switch (tag) {
            case ErrorTag::kEvalErrorPrototype:
                error_type = JERRY_ERROR_EVAL;
                break;
            case ErrorTag::kRangeErrorPrototype:
                error_type = JERRY_ERROR_RANGE;
                break;
            case ErrorTag::kReferenceErrorPrototype:
                error_type = JERRY_ERROR_REFERENCE;
                break;
            case ErrorTag::kSyntaxErrorPrototype:
                error_type = JERRY_ERROR_SYNTAX;
                break;
            case ErrorTag::kTypeErrorPrototype:
                error_type = JERRY_ERROR_TYPE;
                break;
            case ErrorTag::kUriErrorPrototype:
                error_type = JERRY_ERROR_URI;
                break;
            case ErrorTag::kMessage: {
                message = ReadValueInternal();
                break;
            }
            case ErrorTag::kStack: {
                stack = ReadValueInternal();
                break;
            }
            case ErrorTag::kEnd: {
                // TODO: error is not handled properly.
                // Currently it is considered as exception in node
                // and could not be inserted into object
                jerry_value_t result =
                    jerry_create_error(error_type, (const jerry_char_t*)"");
                jerry_value_t prop_name =
                    jerry_create_string((const jerry_char_t*)"message");
                jerry_set_property(result, prop_name, message);
                jerry_release_value(message);
                jerry_release_value(prop_name);
                prop_name = jerry_create_string((const jerry_char_t*)"stack");
                jerry_set_property(result, prop_name, stack);
                jerry_release_value(stack);
                jerry_release_value(prop_name);
                return result;
            }
            default:
                return jerry_create_error(
                    JERRY_ERROR_COMMON,
                    (const jerry_char_t*)"Deserialization error");
        }
    }
    return jerry_create_error(JERRY_ERROR_COMMON,
                              (const jerry_char_t*)"Deserialization error");
}

jerry_value_t ValueDeserializer::ReadJerryObject() {
    jerry_value_t result = jerry_create_object();
    size_t properties_read = 0;
    while (!CheckTag(SerializationTag::kEndJSObject)) {
        jerry_value_t prop_key = ReadValueInternal();
        if (!jerry_value_is_undefined(prop_key)) {
            jerry_value_t prop_value = ReadValueInternal();
            jerry_value_t res =
                jerry_set_property(result, prop_key, prop_value);
            properties_read++;
            jerry_release_value(res);
            jerry_release_value(prop_value);
            jerry_release_value(prop_key);
        } else {
            return jerry_create_undefined();
        }
    }
    SerializationTag tag;
    ReadTag(&tag);
    if (tag == SerializationTag::kEndJSObject) {
        uint32_t properties_written;
        if (ReadVarint<uint32_t>(&properties_written)) {
            if (properties_read == properties_written) {
                return result;
            }
        }
    }
    jerry_release_value(result);
    return jerry_create_undefined();
}

}  // namespace JerrySerialize
