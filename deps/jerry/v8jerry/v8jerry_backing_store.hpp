#ifndef V8JERRY_BACKING_STORE_HPP
#define V8JERRY_BACKING_STORE_HPP

#include <jerryscript.h>
#include <v8.h>

class JerryBackingStore;

class BackingStoreRef {
public:
  BackingStoreRef(JerryBackingStore *ref) : m_backingStore(ref) { }

  JerryBackingStore *m_backingStore;
};

class JerryBackingStore {
public:
    JerryBackingStore(size_t byteLength)
        : m_byteLength(byteLength)
        , m_isShared(false)
    {
        m_data = malloc(byteLength);
        m_deleterData = NULL;
        m_deleter = MallocDeleter;
    }

    // JerryBackingStore(JerryBackingStore *backingStore)
    //     : m_data(backingStore->data())
    //     , m_byteLength(backingStore->byteLength())
    //     , m_deleter(backingStore->deleter())
    //     , m_deleterData(backingStore->deleterData())
    //     , m_isShared(backingStore->isShared())
    // {}

    JerryBackingStore(void* data, size_t byteLength, v8::BackingStoreDeleterCallback deleter = NULL, void* deleterData = NULL, bool isShared = false)
        : m_data(data)
        , m_byteLength(byteLength)
        , m_deleter(deleter)
        , m_deleterData(deleterData)
        , m_isShared(isShared)
    {}

    void* data() const {
        return m_data;
    }

    void setData(void* data) {
        m_data = data;
    }

    size_t byteLength () const {
        return m_byteLength;
    }

    bool isShared() const {
        return m_isShared;
    }

    v8::BackingStoreDeleterCallback deleter() const {
        return m_deleter;
    }

    void* deleterData() const {
        return m_deleterData;
    }

    ~JerryBackingStore() {
        if (m_deleter) {
            m_deleter(m_data, m_byteLength, m_deleterData);
        }
    }

    static void MallocDeleter(void* data, size_t length, void* deleter_data) { free (data); };
    static void EmptyDeleter(void* data, size_t length, void* deleter_data) { };
    static std::unique_ptr<v8::BackingStore> Reallocate(
        v8::Isolate* isolate, std::unique_ptr<v8::BackingStore> backing_store, size_t byte_length);
    //static v8::BackingStore* toV8(JerryBackingStore* backing_store) { return reinterpret_cast<v8::BackingStore*>(backing_store); }
    static const JerryBackingStore* fromV8(const v8::BackingStore* backing_store) { return reinterpret_cast<const BackingStoreRef*>(backing_store)->m_backingStore; }
    static JerryBackingStore* fromV8(v8::BackingStore* backing_store) { return reinterpret_cast<BackingStoreRef*>(backing_store)->m_backingStore; }
private:
    void* m_data;
    void* m_deleterData;
    size_t m_byteLength;
    v8::BackingStoreDeleterCallback m_deleter;
    bool m_isShared;
};

#endif

