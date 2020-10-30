#include "v8jerry_allocator.hpp"

void* JerryAllocator::Allocate(size_t length)
{
  if (length == 0) {
    return (void*)0x1;
  }

  void *data = jerry_heap_alloc(length);

  if (data) {
    memset(data, 0, length);
  }

  return data;
}

void* JerryAllocator::AllocateUninitialized(size_t length)
{
  if (length == 0) {
    return (void*)0x1;
  }

  return jerry_heap_alloc(length);
}


void JerryAllocator::Free(void* data, size_t length)
{
  if (length > 0) {
    jerry_heap_free(data, length);
  }
}


void* JerryAllocator::Reallocate(void* data, size_t old_length, size_t new_length)
{
  // TODO add jerry realloc API

  if (new_length == 0) {
    if (old_length > 0) {
      jerry_heap_free(data, old_length);
    }
    return (void*)0x1;
  }

  void *newData = jerry_heap_alloc(new_length);

  if (newData) {
    memcpy(newData, data, old_length);

    if (old_length > 0) {
      jerry_heap_free(data, old_length);
    }
  }
  return newData;
}

JerryAllocator* JerryAllocator::NewDefaultAllocator()
{
  return new JerryAllocator();
}
