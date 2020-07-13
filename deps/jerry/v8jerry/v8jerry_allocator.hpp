#ifndef V8JERRY_ALLOCATOR_HPP
#define V8JERRY_ALLOCATOR_HPP

#include <jerryscript.h>
#include <v8.h>

class JerryAllocator;

class JerryAllocator {
public:
    JerryAllocator() = default;

    void* Allocate(size_t length);
    void* AllocateUninitialized(size_t length);
    void Free(void* data, size_t length);
    void* Reallocate(void* data, size_t old_length, size_t new_length);

    static JerryAllocator* NewDefaultAllocator();
    static v8::ArrayBuffer::Allocator* toV8(JerryAllocator* alloc) { return reinterpret_cast<v8::ArrayBuffer::Allocator*>(alloc); }
    static JerryAllocator* fromV8(v8::ArrayBuffer::Allocator* alloc) { return reinterpret_cast<JerryAllocator*>(alloc); }
};

#endif

