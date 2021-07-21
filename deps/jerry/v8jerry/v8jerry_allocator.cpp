#include "v8jerry_allocator.hpp"

void* JerryAllocator::Allocate(size_t length)
{
    if (length == 0) {
        return (void*)0x1;
    }

    return calloc(length, 1);
}

void* JerryAllocator::AllocateUninitialized(size_t length)
{
    if (length == 0) {
        return (void*)0x1;
    }

    return malloc(length);
}


void JerryAllocator::Free(void* data, size_t length)
{
    if (length > 0 || (data != NULL && data != (void*)0x1)) {
        free(data);
    }
}

void* JerryAllocator::Reallocate(void* data, size_t old_length, size_t new_length)
{
    // TODO add jerry realloc API
    if (new_length == 0) {
        if (old_length > 0) {
           free(data);
        }
        return (void*)0x1;
    }

    if (data == (void*)0x1) {
        return malloc(new_length);
    }

    return realloc(data, new_length);
}

JerryAllocator* JerryAllocator::NewDefaultAllocator()
{
    return new JerryAllocator();
}
