/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __TizenDeviceAPILoader__
#define __TizenDeviceAPILoader__

#include <mutex>
#include <map>
#include "v8.h"
#include <dlog.h>

#undef LOGGER_TAG
#define LOGGER_TAG "NodeDeviceAPI"

#ifndef __MODULE__
#define __MODULE__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define _LOGGER_LOG(prio, fmt, args...)                                   \
  dlog_print(prio, LOGGER_TAG, "%s: %s(%d) > " fmt, __MODULE__, __func__, \
             __LINE__, ##args);

#define _LOGGER_SLOG(prio, fmt, args...)                                      \
  SECURE_LOG_(LOG_ID_MAIN, prio, LOGGER_TAG, "%s: %s(%d) > " fmt, __MODULE__, \
              __func__, __LINE__, ##args);

#define DEVICEAPI_LOG_INFO(fmt, args...) _LOGGER_SLOG(DLOG_INFO, fmt, ##args)
#define DEVICEAPI_LOG_ERROR(fmt, args...) \
  _LOGGER_SLOG(DLOG_ERROR, "Error: " fmt, ##args)
#define DEVICEAPI_LOG_WARN(fmt, args...) _LOGGER_SLOG(DLOG_WARN, fmt, ##args)

#define DEVICEAPI_SLOG_INFO(fmt, args...) _LOGGER_SLOG(DLOG_INFO, fmt, ##args)
#define DEVICEAPI_SLOG_ERROR(fmt, args...) _LOGGER_SLOG(DLOG_ERROR, fmt, ##args)
#define DEVICEAPI_SLOG_WARN(fmt, args...) _LOGGER_SLOG(DLOG_WARN, fmt, ##args)
#define VALUE_NAME_STRCAT(name) name##Value

#if defined(DEBUG)
#include <assert.h>
#define DEVICEAPI_ASSERT(assertion) assert(assertion)
#else
#define DEVICEAPI_ASSERT(assertion)                                           \
  do {                                                                        \
    if (!assertion) {                                                         \
      DEVICEAPI_LOG_ERROR("ASSERT_SHOULD_NOT_BE_HERE at %s (%d)\n", __FILE__, \
                          __LINE__);                                          \
      ::abort();                                                              \
    }                                                                         \
  } while (0)
#endif

namespace wrt {
namespace xwalk {
class Extension;
class ExtensionInstance;
}  // namespace xwalk
}  // namespace wrt

namespace DeviceAPI {

class ESPostListener;

#define FOR_EACH_EARLY_TIZEN_STRINGS(F) \
  F(tizen)                              \
  F(xwalk)                              \
  F(webapis)

#define FOR_EACH_LAZY_TIZEN_STRINGS(F) \
  F(utils)                             \
  F(common)                            \
  F(extension)                         \
  F(postMessage)                       \
  F(sendSyncMessage)                   \
  F(sendSyncData)                      \
  F(sendRuntimeMessage)                \
  F(sendRuntimeSyncMessage)            \
  F(sendRuntimeAsyncMessage)           \
  F(setMessageListener)                \
  F(receiveChunkData)                  \
  F(reply)                             \
  F(chunk_id)                          \
  F(string)                            \
  F(octet)

#define SUPPORTED_TIZEN_PROPERTY(F) \
  F(application)                    \
  F(filesystem)                     \
  F(mediacontroller)                \
  F(messageport)                    \
  F(systeminfo)                     \
  F(sensorservice)                  \
  F(tvaudiocontrol)                 \
  F(preference)                     \
  F(power)                          \
  F(time)

#define SUPPORTED_TIZEN_ENTRYPOINTS(F) \
  F(ApplicationControl)                \
  F(ApplicationControlData)

/*
 * Extension: (tizen, utils, common, messageport, sensorservice...) * 1
 * ExtensionManager: (manager) * 1
 * ExtensionInstance: (tizen, utils, common, messageport, sensorservice...) *
 * number of ESVMInstances
 * ExtensionManagerInstance: (manager) * number of ESVMInstances
 */

class ExtensionManagerInstance {
 public:
  ExtensionManagerInstance(v8::Local<v8::Context> context);
  ~ExtensionManagerInstance();

  static ExtensionManagerInstance* getExtensionInstance(
      v8::Local<v8::Context> context);
  static wrt::xwalk::ExtensionInstance* getExtensionInstanceFromCallingObject(
      v8::Local<v8::Value> thisValue);
  v8::Local<v8::Object> initializeExtensionInstance(const char*);
  v8::Local<v8::Context> Context() { return m_context; }

  v8::Persistent<v8::Value> m_tizenValue;
  v8::Persistent<v8::Value> m_xwalkValue;

  v8::Local<v8::ObjectTemplate> m_extensionObjectTemplate;

 private:
  struct ChunkData {
    ChunkData() {}
    ChunkData(uint8_t* buffer, size_t length)
        : m_buffer(buffer), m_length(length) {}
    uint8_t* m_buffer;
    size_t m_length;
  };

  typedef std::map<size_t, ChunkData> ChunkDataMap;
  typedef std::map<v8::Object*, wrt::xwalk::ExtensionInstance*>
      ExtensionInstanceMap;
  typedef std::vector<ESPostListener*> ESPostListenerVector;

  v8::Local<v8::Object> createExtensionObject();
  size_t addChunk(uint8_t* buffer, size_t length);
  ChunkData getChunk(size_t chunkID);

  v8::Local<v8::Context> m_context;
  ExtensionInstanceMap m_extensionInstances;
  ESPostListenerVector m_postListeners;
  ChunkDataMap m_chunkDataMap;
  size_t m_chunkID;

  // static members
  typedef std::map<v8::Context*, ExtensionManagerInstance*>
      ExtensionManagerInstanceMap;
  static wrt::xwalk::Extension* getExtension(const char* apiName);
  static ExtensionManagerInstanceMap s_extensionManagerInstances;
  static std::mutex s_mutex;
};

inline ExtensionManagerInstance* ExtensionManagerInstanceGet(
    v8::Local<v8::Context> context) {
  return ExtensionManagerInstance::getExtensionInstance(context);
}

ExtensionManagerInstance* initialize(v8::Local<v8::Context> context);
void close(v8::Local<v8::Context> context);
}  // namespace DeviceAPI

#endif  // __TizenDeviceAPILoader__
