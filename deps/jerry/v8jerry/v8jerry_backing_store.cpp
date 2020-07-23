#include "v8jerry_backing_store.hpp"

std::unique_ptr<v8::BackingStore> JerryBackingStore::Reallocate(
  v8::Isolate* isolate, std::unique_ptr<v8::BackingStore> backing_store,
  size_t byte_length) {

  JerryBackingStore *jbackingStore = JerryBackingStore::fromV8(backing_store.get());
  void *new_data = realloc(jbackingStore->data(), byte_length);
  jbackingStore->setData(new_data);

  return backing_store;
}
