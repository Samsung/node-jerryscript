#ifndef V8JERRY_SERIALIZE_HPP
#define V8JERRY_SERIALIZE_HPP

#include "v8.h"
#include "v8jerry_value.hpp"

namespace i = v8::internal;

namespace JerrySerialize {

enum class SerializationTag : uint8_t;
enum class ErrorTag : uint8_t;

class SerializerBuffer {
   public:
    SerializerBuffer() : buffer_((uint8_t*)malloc(4)), capacity_(4), size_(0) {}
    SerializerBuffer(uint8_t* buffer, size_t size)
        : buffer_(buffer), capacity_(size_), size_(size) {}
    ~SerializerBuffer() {
        if (buffer_) {
            free(buffer_);
        }
    }

    size_t Size() { return size_; }

    void Append(const void* source, size_t length) {
        Expand(size_ + length);
        memcpy(buffer_ + size_, source, length);
        size_ += length;
    }

    uint8_t* Release() {
        size_ = 0;
        capacity_ = 0;
        uint8_t* ret = buffer_;
        buffer_ = nullptr;
        return ret;
    }

    uint8_t& operator[](size_t pos) { return buffer_[pos]; }

   private:
    void Expand(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        while (new_capacity > capacity_) {
            capacity_ = capacity_ * 2;
        }
        buffer_ = (uint8_t*)realloc(buffer_, capacity_);
    }

    uint8_t* buffer_;
    size_t capacity_;
    size_t size_;
};

class ValueSerializer {
   public:
    ValueSerializer(i::Isolate* isolate,
                    v8::ValueSerializer::Delegate* delegate);
    ~ValueSerializer(){};

    void WriteHeader();
    bool WriteValue(JerryValue* value);
    void WriteUint32(uint32_t value);
    void WriteUint64(uint64_t value);
    void WriteDouble(double value);
    void WriteRawBytes(const void* source, size_t length);
    std::pair<uint8_t*, size_t> Release();
    void SetTreatArrayBufferViewsAsHostObjects(bool mode);

   private:
    bool WriteValueInternal(jerry_value_t value);
    void WriteTag(SerializationTag tag);
    template <typename T>
    void WriteVarint(T value);
    template <typename T>
    void WriteZigZag(T value);
    bool WriteJerryArray(jerry_value_t value);
    bool WriteJerryArrayBuffer(jerry_value_t value);
    void WriteJerryString(jerry_value_t value);
    void WriteJerryError(jerry_value_t value);
    bool WriteJerryObject(jerry_value_t value);

    SerializerBuffer buffer_;
    i::Isolate* const isolate_;
    v8::ValueSerializer::Delegate* const delegate_;

    bool treat_array_buffer_views_as_host_objects_;
};

class ValueDeserializer {
   public:
    ValueDeserializer(i::Isolate* isolate,
                      const uint8_t* data,
                      const size_t size,
                      v8::ValueDeserializer::Delegate* delegate);
    ~ValueDeserializer(){};

    bool ReadHeader();
    JerryValue* ReadValue();
    bool ReadDouble(double* value);
    bool ReadUint32(uint32_t* value);
    bool ReadUint64(uint64_t* value);

    bool ReadRawBytes(size_t length, const void** data);

   private:
    jerry_value_t ReadValueInternal();
    bool ReadTag(SerializationTag* tag);
    bool ReadErrorTag(ErrorTag* tag);
    bool CheckTag(SerializationTag check);

    template <typename T>
    bool ReadVarint(T* value);
    template <typename T>
    bool ReadZigZag(T* value);
    bool ReadOneByteString(JerryString* value);

    jerry_value_t ReadJerryArray();
    jerry_value_t ReadJerryArrayBuffer();
    jerry_value_t ReadJerryError();
    jerry_value_t ReadJerryObject();

    const uint8_t* buffer_;
    size_t position_;
    const size_t size_;
    i::Isolate* const isolate_;
    v8::ValueDeserializer::Delegate* const delegate_;
};

}  // namespace JerrySerialize

#endif
