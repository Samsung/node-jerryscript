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

#include "TizenDeviceAPILoader.h"

#include <dlfcn.h>
#include <string.h>

#include "ExtensionAdapter.h"
#include "ExtensionManager.h"

using namespace v8;

namespace DeviceAPI {

wrt::xwalk::Extension* ExtensionManagerInstance::getExtension(
    const char* apiName) {
  DEVICEAPI_LOG_INFO("Creating a new extension: %s\n", apiName);
  auto extensionManager = wrt::xwalk::ExtensionManager::GetInstance();
  wrt::xwalk::ExtensionMap& extensions = extensionManager->extensions();

  auto it = extensions.find(apiName);
  if (it == extensions.end()) {
    DEVICEAPI_LOG_INFO("Enter");
    char library_path[512];
    if (!strcmp(apiName, "tizen")) {
      snprintf(library_path, 512,
               "/usr/lib/tizen-extensions-crosswalk/libtizen.so");
    } else if (!strcmp(apiName, "sensorservice")) {
      snprintf(library_path, 512,
               "/usr/lib/tizen-extensions-crosswalk/libtizen_sensor.so");
    } else if (!strcmp(apiName, "sa")) {
      snprintf(library_path, 512,
               "/usr/lib/tizen-extensions-crosswalk/libwebapis_sa.so");
    } else if (!strcmp(apiName, "tvaudiocontrol")) {
      snprintf(library_path, 512,
               "/usr/lib/tizen-extensions-crosswalk/libtizen_tvaudio.so");
    } else {
      snprintf(library_path, 512,
               "/usr/lib/tizen-extensions-crosswalk/libtizen_%s.so", apiName);
    }

    wrt::xwalk::Extension* extension =
        new wrt::xwalk::Extension(library_path, extensionManager);

    if (extension->Initialize()) {
      extensionManager->RegisterExtension(extension);
      extensions[apiName] = extension;
      return extension;
    } else {
      DEVICEAPI_LOG_INFO("Cannot initialize extension %s", apiName);
      return nullptr;
    }
  } else {
    return it->second;
  }
}

// Caution: this function is called only inside existing js execution context,
// so we don't handle JS exception around ESFunctionObject::call()
Local<Object> ExtensionManagerInstance::initializeExtensionInstance(
    const char* apiName) {
  DEVICEAPI_LOG_INFO("Enter");

  // TODO: implement

  return Local<Object>();
}

Local<Object> ExtensionManagerInstance::createExtensionObject(
    v8::Local<v8::Context> context) {
  DEVICEAPI_LOG_INFO("Enter");

  // TODO: implement

  return Local<Object>();
}

wrt::xwalk::ExtensionInstance*
ExtensionManagerInstance::getExtensionInstanceFromCallingContext(
    v8::Local<v8::Context> context, v8::Local<v8::Value> thisValue) {
  DEVICEAPI_LOG_INFO("Enter");

  // TODO: implement

  return nullptr;
}

size_t ExtensionManagerInstance::addChunk(uint8_t* buffer, size_t length) {
  DEVICEAPI_LOG_INFO("Enter");
  size_t chunkID = m_chunkID++;
  m_chunkDataMap[chunkID] = ChunkData(buffer, length);
  return chunkID;
}

ExtensionManagerInstance::ChunkData ExtensionManagerInstance::getChunk(
    size_t chunkID) {
  DEVICEAPI_LOG_INFO("Enter");
  auto it = m_chunkDataMap.find(chunkID);
  if (it == m_chunkDataMap.end()) {
    return ChunkData(nullptr, 0);
  } else {
    ChunkData chunkData = it->second;
    m_chunkDataMap.erase(it);
    return chunkData;
  }
}

ExtensionManagerInstance::ExtensionManagerInstanceMap
    ExtensionManagerInstance::s_extensionManagerInstances;

std::mutex ExtensionManagerInstance::s_mutex;

ExtensionManagerInstance::ExtensionManagerInstance(
    v8::Local<v8::Context> context)
    : m_context(context), m_chunkID(0) {
  DEVICEAPI_LOG_INFO("new ExtensionManagerInstance %p", this);

#define DECLARE_TIZEN_OBJECT(name) VALUE_NAME_STRCAT(m_##name) = Local<Value>();
  FOR_EACH_EARLY_TIZEN_STRINGS(DECLARE_TIZEN_OBJECT);
  FOR_EACH_LAZY_TIZEN_STRINGS(DECLARE_TIZEN_OBJECT);
  SUPPORTED_TIZEN_PROPERTY(DECLARE_TIZEN_OBJECT);
  SUPPORTED_TIZEN_ENTRYPOINTS(DECLARE_TIZEN_OBJECT);
#undef DECLARE_TIZEN_OBJECT

  // TODO
}

ExtensionManagerInstance::~ExtensionManagerInstance() {
  std::lock_guard<std::mutex> guard(s_mutex);
  DEVICEAPI_LOG_INFO(
      "ExtensionManagerInstance delete ExtensionManagerInstance %p", this);
  for (auto it : m_extensionInstances) delete it.second;
  for (auto it : m_postListeners) it->finalize();
  auto it = s_extensionManagerInstances.find(*m_context);
  s_extensionManagerInstances.erase(it);
  DEVICEAPI_LOG_INFO("ExtensionManagerInstance %zu => %zu",
                     s_extensionManagerInstances.size() + 1,
                     s_extensionManagerInstances.size());
}

ExtensionManagerInstance* ExtensionManagerInstance::get(
    v8::Local<v8::Context> context) {
  std::lock_guard<std::mutex> guard(s_mutex);
  ExtensionManagerInstance* instance = nullptr;

  auto it = s_extensionManagerInstances.find(*context);
  if (it != s_extensionManagerInstances.end()) {
    instance = it->second;
  }

  return instance;
}

ExtensionManagerInstance* initialize(v8::Local<v8::Context> context) {
  DEVICEAPI_LOG_INFO("ExtensionManagerInstance Enter with context %p", context);
  return new ExtensionManagerInstance(context);
}

void close(v8::Local<v8::Context> context) {
  DEVICEAPI_LOG_INFO("ExtensionManagerInstance Enter with context %p", context);
  delete ExtensionManagerInstance::get(context);
}
}  // namespace DeviceAPI
