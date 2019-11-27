#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "v8env.h"
#include "assert.h"

#define ARRAY_SIZE 8

int main(int argc, char* argv[]) {
    // Initialize V8.
    V8Environment env(argc, argv);

    //
    // Array
    //

    // Create a basic array.
    v8::Local<v8::Array> array = v8::Array::New(env.getIsolate(), 3);

    ASSERT_EQUAL(array->Length(), 3);

    //
    // ArrayBuffer
    //

    uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * ARRAY_SIZE);
    memset(data, 0, ARRAY_SIZE);
    data[0] = 1;
    data[1] = 1;

    // Create zero length arraybuffer.
    v8::Local<v8::ArrayBuffer> zeroBuffer =
        v8::ArrayBuffer::New(env.getIsolate(), NULL, 0, v8::ArrayBufferCreationMode::kInternalized);
    ASSERT_EQUAL(zeroBuffer->ByteLength(), 0);

    // Create an internalized arraybuffer.
    v8::Local<v8::ArrayBuffer> intBuffer =
        v8::ArrayBuffer::New(env.getIsolate(), (void*) data, ARRAY_SIZE, v8::ArrayBufferCreationMode::kInternalized);

    // Get its contents.
    v8::ArrayBuffer::Contents contents = intBuffer->GetContents();
    // Get its size.
    size_t byteLength = intBuffer->ByteLength();
    // Check the size of the array.
    ASSERT_EQUAL(byteLength, ARRAY_SIZE);

    // Do a modification in the original array.
    data[0] = 0;
    // Check data similarity.
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ASSERT_EQUAL(data[i], *((uint8_t*)contents.Data() + i));
    }
    // Check the size of the array.
    ASSERT_EQUAL(byteLength, contents.ByteLength());
    // Check if the value is ArrayBuffer,
    ASSERT_EQUAL(intBuffer->IsArrayBuffer(), true);

    // Create an externalized arraybuffer.
    v8::Local<v8::ArrayBuffer> extBuffer =
        v8::ArrayBuffer::New(env.getIsolate(), (void*) data, ARRAY_SIZE, v8::ArrayBufferCreationMode::kExternalized);

    // Get its contents.
    contents = extBuffer->GetContents();
    // Get its size.
    byteLength = extBuffer->ByteLength();
    // Check the size of the array.
    ASSERT_EQUAL(byteLength, 8);

    // Do a modification in the original array.
    data[0] = 1;
    // Check data similarity.
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ASSERT_EQUAL(data[i], *((uint8_t*)contents.Data() + i));
    }
    // Check the size of the array.
    ASSERT_EQUAL(byteLength, contents.ByteLength());
    // Check if the value is ArrayBuffer.
    ASSERT_EQUAL(extBuffer->IsArrayBuffer(), true);

    //
    // ArraybufferView
    //

    // Create Uint8 viewer onto the ArrayBuffer.
    v8::Local<v8::Uint8Array> uint8Array = v8::Uint8Array::New(intBuffer, 2, 5);
    ASSERT_EQUAL(uint8Array->ByteOffset(), 2);
    ASSERT_EQUAL(uint8Array->ByteLength(), 5);
    // Create Uint32 viewer onto the ArrayBuffer.
    v8::Local<v8::Uint32Array> uint32Array = v8::Uint32Array::New(intBuffer, 0, 1);
    ASSERT_EQUAL(uint32Array->ByteOffset(), 0);
    ASSERT_EQUAL(uint32Array->ByteLength(), 4);
    // Create Float64 viewer onto the ArrayBuffer.
    v8::Local<v8::Float64Array> float64Array = v8::Float64Array::New(intBuffer, 0, 1);
    ASSERT_EQUAL(float64Array->ByteOffset(), 0);
    ASSERT_EQUAL(float64Array->ByteLength(), 8);

    // Check data similarity for Uint8Array.
    for (int i = uint8Array->ByteOffset(); i < uint8Array->ByteLength(); i++) {
        ASSERT_EQUAL(data[i], *((uint8_t*)uint8Array->Buffer()->GetContents().Data() + i));
    }
    // Check data similarity for Uint32Array.
    ASSERT_EQUAL(*((uint32_t*)uint8Array->Buffer()->GetContents().Data()), 257);

    ASSERT_EQUAL(uint8Array->IsUint8Array(), true);
    ASSERT_EQUAL(float64Array->IsFloat64Array(), true);

    return 0;
}
