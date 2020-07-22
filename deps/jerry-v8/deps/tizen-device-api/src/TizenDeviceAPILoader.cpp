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
#include "v8utils.h"

using namespace v8;

namespace DeviceAPI {

wrt::xwalk::Extension* ExtensionManagerInstance::getExtension(
    const char* apiName) {
  DEVICEAPI_LOG_INFO("Creating a new extension: %s", apiName);
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

static std::string CreateExtensionInternalFunctionSource(
    wrt::xwalk::Extension* extension, const char* apiName) {
  std::string str;
  str.append(R"(
                   (function(extension) {
                       console.log("[JS extension2]");
                       console.log(extension);
                       extension.internal = {};
                       extension.internal.sendSyncMessage_ = extension.sendSyncMessage;
                       extension.internal.sendSyncMessage = function() {
                           return extension.internal.sendSyncMessage_.apply(extension, arguments);
                       };
                       delete extension.sendSyncMessage;
                       var exports = {};
                       var window = {
                           Object,
                       };
                   )");
#ifdef DEBUG
  str.append("console.log('[JS] Start loading ");
  str.append(apiName);
  str.append("');");
#endif
  str.append("(function() {");
  str.append(extension->javascript_api().c_str());
  str.append("})();");
#ifdef DEBUG
  str.append("console.log('[JS] Loading ");
  str.append(apiName);
  str.append(" done ');");
#endif
  str.append("return exports;})");

  return str;
}

// Caution: this function is called only inside existing js execution context,
// so we don't handle JS exception around ESFunctionObject::call()
Local<Object> ExtensionManagerInstance::initializeExtensionInstance(
    const char* apiName) {
  DEVICEAPI_LOG_INFO("Enter");

  wrt::xwalk::Extension* extension = getExtension(apiName);
  if (!extension) {
    DEVICEAPI_LOG_INFO("Cannot load extension %s", apiName);
    return Local<Object>();
  }

  std::string source =
      CreateExtensionInternalFunctionSource(extension, apiName);

  Local<Value> compile = v8utils::CompileRun(m_context, source.c_str());
  DEVICEAPI_ASSERT(compile->IsFunction());
  Local<Function> initializer = compile.As<Function>();

  Local<Object> extensionObject = createExtensionObject();
  wrt::xwalk::ExtensionInstance* extensionInstance =
      extension->CreateInstance();

  extensionObject->SetAlignedPointerInInternalField(0, extensionInstance);
  Local<Value> args[] = {extensionObject};

  TryCatch try_catch(m_context->GetIsolate());
  MaybeLocal<Value> ret =
      initializer->Call(m_context, Undefined(m_context->GetIsolate()), 1, args);
  if (ret.IsEmpty()) {
    DEVICEAPI_LOG_ERROR("Fail to run script(%s)", apiName);
    return Local<Object>();
  }
  DEVICEAPI_LOG_INFO("Done");
  return ret.ToLocalChecked()->ToObject(m_context).ToLocalChecked();
}

Local<Object> ExtensionManagerInstance::createExtensionObject() {
  DEVICEAPI_LOG_INFO("Enter");
  Local<Object> extensionObject =
      m_extensionObjectTemplate->NewInstance(m_context).ToLocalChecked();

  // postMessage
  Local<Function> postMessageFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_ERROR("extension.postMessage UNIMPLEMENTED");
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "postMessage"), postMessageFn);

  // sendSyncMessage
  Local<Function> sendSyncMessageFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_INFO("extension.sendSyncMessage");

        Isolate* isolate = info.GetIsolate();
        wrt::xwalk::ExtensionInstance* extensionInstance =
            getExtensionInstanceFromCallingObject(info.This());
        if (!extensionInstance || info.Length() != 1) {
          DEVICEAPI_LOG_ERROR("extensionInstance == nullptr");
          info.GetReturnValue().Set(False(isolate));
          return;
        }

        String::Utf8Value message(isolate, info[0]);
        std::string stdStringMessage(*message);
        extensionInstance->HandleSyncMessage(stdStringMessage);

        std::string reply = extensionInstance->sync_replay_msg();
        DEVICEAPI_LOG_INFO("extension.sendSyncMessage Done with reply %s",
                           reply.c_str());

        if (reply.empty()) {
          info.GetReturnValue().Set(Null(isolate));
        }
        Local<String> replyString =
            String::NewFromUtf8(isolate, reply.c_str(), String::kNormalString);
        info.GetReturnValue().Set(replyString);
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "sendSyncMessage"),
      sendSyncMessageFn);

  // sendSyncData
  Local<Function> sendSyncDataFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_INFO("extension.sendSyncData");

        Isolate* isolate = info.GetIsolate();
        Local<v8::Context> context = isolate->GetCurrentContext();
        ExtensionManagerInstance* extensionManagerInstance =
            getExtensionInstance(context);

        wrt::xwalk::ExtensionInstance* extensionInstance =
            getExtensionInstanceFromCallingObject(info.This());
        if (!extensionInstance || info.Length() < 1) {
          info.GetReturnValue().Set(False(isolate));
        }

        ChunkData chunkData(nullptr, 0);
        if (info.Length() > 1) {
          Local<Value> dataValue = info[1];
          if (dataValue->IsObject()) {
            Local<Object> arrayData = dataValue->ToObject(isolate);
            size_t length =
                arrayData->Get(context, v8utils::ToStr(context, "length"))
                    .ToLocalChecked()
                    ->ToInteger(isolate)
                    ->Value();
            uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * length);
            for (size_t i = 0; i < length; i++) {
              buffer[i] = arrayData->Get(context, v8utils::ToNum(context, i))
                              .ToLocalChecked()
                              ->ToInteger(isolate)
                              ->Value();
            }
            chunkData = ChunkData(buffer, length);
          } else if (dataValue->IsString()) {
            String::Utf8Value stringData(
                dataValue->ToString(context).ToLocalChecked());
            chunkData = ChunkData((uint8_t*)(*stringData), stringData.length());
          }
        }

        String::Utf8Value message(info[0]->ToString(context).ToLocalChecked());
        extensionInstance->HandleSyncData(*message, chunkData.m_buffer,
                                          chunkData.m_length);

        uint8_t* replyBuffer = nullptr;
        size_t replyLength = 0;
        std::string reply =
            extensionInstance->sync_data_reply_msg(&replyBuffer, &replyLength);

        DEVICEAPI_LOG_INFO(
            "extension.sendSyncData Done with reply %s (buffer %s)",
            reply.c_str(), replyBuffer);

        if (reply.empty()) {
          info.GetReturnValue().Set(Null(isolate));
        }

        Local<Object> returnObject = Object::New(isolate);
        returnObject->DefineOwnProperty(context,
                                        v8utils::ToStr(context, "reply"),
                                        v8utils::ToStr(context, reply.c_str()));

        if (replyBuffer || replyLength > 0) {
          size_t chunkID =
              extensionManagerInstance->addChunk(replyBuffer, replyLength);
          returnObject->DefineOwnProperty(context,
                                          v8utils::ToStr(context, "chunk_id"),
                                          v8utils::ToNum(context, chunkID));
        }
        info.GetReturnValue().Set(returnObject);
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "sendSyncData"), sendSyncDataFn);

  // sendRuntimeMessage
  Local<Function> sendRuntimeMessageFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_ERROR("extension.sendRuntimeMessage UNIMPLEMENTED");
        info.GetReturnValue().Set(Undefined(info.GetIsolate()));
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "sendRuntimeMessage"),
      sendRuntimeMessageFn);

  // sendRuntimeAsyncMessage
  Local<Function> sendRuntimeAsyncMessageFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_ERROR("extension.sendRuntimeAsyncMessage UNIMPLEMENTED");
        info.GetReturnValue().Set(Undefined(info.GetIsolate()));
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "sendRuntimeAsyncMessage"),
      sendRuntimeAsyncMessageFn);

  // sendRuntimeSyncMessage
  Local<Function> sendRuntimeSyncMessageFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_ERROR("extension.sendRuntimeSyncMessage UNIMPLEMENTED");
        info.GetReturnValue().Set(Undefined(info.GetIsolate()));
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "sendRuntimeSyncMessage"),
      sendRuntimeSyncMessageFn);

  // setMessageListener
  Local<Function> setMessageListenerFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_INFO("extension.setMessageListener");

        wrt::xwalk::ExtensionInstance* extensionInstance =
            getExtensionInstanceFromCallingObject(info.This());
        Isolate* isolate = info.GetIsolate();
        Local<v8::Context> context = isolate->GetCurrentContext();
        ExtensionManagerInstance* extensionManagerInstance =
            getExtensionInstance(context);

        if (!extensionInstance || info.Length() != 1) {
          info.GetReturnValue().Set(False(isolate));
          return;
        }

        Local<Value> listenerValue = info[0];
        if (listenerValue->IsUndefined()) {
          DEVICEAPI_LOG_ERROR("listenerValue == undefined");
          extensionInstance->set_post_message_listener(nullptr);
          info.GetReturnValue().Set(True(isolate));
          return;
        }

        if (!listenerValue->IsObject()) {
          DEVICEAPI_LOG_ERROR("Invalid message listener.");
          info.GetReturnValue().Set(False(isolate));
          return;
        }

        ESPostMessageListener* postMessageListener =
            ESPostMessageListener::create(context,
                                          listenerValue->ToObject(isolate));
        extensionInstance->set_post_message_listener(postMessageListener);

        extensionManagerInstance->m_postListeners.push_back(
            postMessageListener);

        info.GetReturnValue().Set(True(isolate));
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "setMessageListener"),
      setMessageListenerFn);

  // receiveChunkData
  Local<Function> receiveChunkDataFn =
      Function::New(m_context, [](const FunctionCallbackInfo<Value>& info) {
        DEVICEAPI_LOG_INFO("extension.receiveChunkData");

        wrt::xwalk::ExtensionInstance* extensionInstance =
            getExtensionInstanceFromCallingObject(info.This());
        Isolate* isolate = info.GetIsolate();
        Local<v8::Context> context = isolate->GetCurrentContext();
        ExtensionManagerInstance* extensionManagerInstance =
            getExtensionInstance(context);

        if (!extensionInstance || info.Length() < 1) {
          info.GetReturnValue().Set(False(isolate));
          return;
        }

        size_t chunkID = info[0]->ToNumber(isolate)->Value();
        ;
        ExtensionManagerInstance::ChunkData chunkData =
            extensionManagerInstance->getChunk(chunkID);
        if (!chunkData.m_buffer) {
          info.GetReturnValue().Set(Null(isolate));
          return;
        }

        String::Utf8Value type(isolate, info[1]);
        bool isStringType = strncmp("octet", *type, strlen("octet")) != 0;

        Local<Value> ret;
        if (isStringType) {
          ret = v8utils::ToStr(context, (char*)chunkData.m_buffer);
        } else {
          Local<Array> octetArray = Array::New(isolate);
          for (size_t i = 0; i < chunkData.m_length; i++) {
            octetArray->Set(context, i,
                            v8utils::ToNum(context, chunkData.m_buffer[i]));
          }
          ret = octetArray;
        }
        free(chunkData.m_buffer);
        info.GetReturnValue().Set(ret);
      }).ToLocalChecked();
  extensionObject->DefineOwnProperty(
      m_context, v8utils::ToStr(m_context, "receiveChunkData"),
      receiveChunkDataFn);

  return extensionObject;
}

wrt::xwalk::ExtensionInstance*
ExtensionManagerInstance::getExtensionInstanceFromCallingObject(
    Local<Value> thisValue) {
  DEVICEAPI_LOG_INFO("Enter");
  if (thisValue->IsNullOrUndefined() || !thisValue->IsObject()) {
    DEVICEAPI_LOG_ERROR("Cannot find ExtensionInstance!");
    return nullptr;
  }

  DEVICEAPI_ASSERT(thisValue->IsObject());
  Local<Object> thisObject = thisValue.As<Object>();
  wrt::xwalk::ExtensionInstance* instance =
      reinterpret_cast<wrt::xwalk::ExtensionInstance*>(
          thisObject->GetAlignedPointerFromInternalField(0));

  return instance;
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

static void SetTizenProperty(Local<Object> tizenObject,
                             ExtensionManagerInstance* extensionManagerInstance,
                             const char* name) {
  Local<Context> context = extensionManagerInstance->Context();
  tizenObject->SetAccessor(
      context, v8utils::ToStr(context, name),
      [](Local<Name> name, const PropertyCallbackInfo<Value>& info) {
        DEVICEAPI_ASSERT(info.Data()->IsExternal());
        ExtensionManagerInstance* extensionManagerInstance =
            reinterpret_cast<ExtensionManagerInstance*>(
                info.Data().As<External>()->Value());
        Local<Context> context = extensionManagerInstance->Context();
        String::Utf8Value propertyKey(name->ToString(context).ToLocalChecked());

        Local<Object> apiObject =
            extensionManagerInstance->initializeExtensionInstance(*propertyKey);

        enum PropertyAttribute attributes =
            static_cast<PropertyAttribute>(ReadOnly | DontDelete);
        extensionManagerInstance->m_tizenValue.Get(info.GetIsolate())
            ->ToObject(context)
            .ToLocalChecked()
            ->DefineOwnProperty(context, v8utils::ToStr(context, *propertyKey),
                                apiObject, attributes);

        info.GetReturnValue().Set(apiObject);
      },
      NULL, External::New(context->GetIsolate(), extensionManagerInstance));
}

static void TizenGetter(Local<Name> name,
                        const PropertyCallbackInfo<Value>& info) {
  DEVICEAPI_ASSERT(info.Data()->IsExternal());
  ExtensionManagerInstance* manager =
      reinterpret_cast<ExtensionManagerInstance*>(
          info.Data().As<External>()->Value());

  Local<Context> context = manager->Context();
  ExtensionManagerInstance* extensionManagerInstance =
      manager->getExtensionInstance(context);

  if (!extensionManagerInstance->m_tizenValue.IsEmpty()) {
    info.GetReturnValue().Set(extensionManagerInstance->m_tizenValue);
    return;
  }
  Local<Object> tizenObject =
      extensionManagerInstance->initializeExtensionInstance("tizen");
  extensionManagerInstance->m_tizenValue.Reset(info.GetIsolate(), tizenObject);

#define SET_TIZENVALUE_PROPERTY(name) \
  SetTizenProperty(tizenObject, manager, #name);
  SUPPORTED_TIZEN_PROPERTY(SET_TIZENVALUE_PROPERTY)
#undef SET_TIZENVALUE_PROPERTY

  info.GetReturnValue().Set(extensionManagerInstance->m_tizenValue);
}

static void XwalkGetter(Local<Name> name,
                        const PropertyCallbackInfo<Value>& info) {
  DEVICEAPI_LOG_INFO("xwalkGetter Enter");
  DEVICEAPI_ASSERT(info.Data()->IsExternal());
  ExtensionManagerInstance* manager =
      reinterpret_cast<ExtensionManagerInstance*>(
          info.Data().As<External>()->Value());

  Local<Context> context = manager->Context();
  ExtensionManagerInstance* extensionManagerInstance =
      manager->getExtensionInstance(context);
  if (!extensionManagerInstance->m_xwalkValue.IsEmpty()) {
    info.GetReturnValue().Set(extensionManagerInstance->m_xwalkValue);
    return;
  }

  DEVICEAPI_LOG_INFO("Loading plugin for xwalk.utils");

  // initialize xwalk object
  Local<Object> xwalkObject =
      extensionManagerInstance->initializeExtensionInstance("utils");
  extensionManagerInstance->m_xwalkValue.Reset(info.GetIsolate(), xwalkObject);

  info.GetReturnValue().Set(extensionManagerInstance->m_xwalkValue);
}

ExtensionManagerInstance::ExtensionManagerInstance(Local<v8::Context> context)
    : m_context(context), m_chunkID(0) {
  DEVICEAPI_LOG_INFO("new ExtensionManagerInstance %p", this);

  HandleScope handleScope(m_context->GetIsolate());
  Context::Scope contextScope(m_context);

  enum PropertyAttribute attributes =
      static_cast<PropertyAttribute>(ReadOnly | DontDelete);
  m_context->Global()->SetAccessor(
      m_context, v8utils::ToStr(m_context, "tizen"), TizenGetter, NULL,
      External::New(m_context->GetIsolate(), this), DEFAULT, attributes);

  // Create xwalk Object
  m_context->Global()->SetAccessor(
      m_context, v8utils::ToStr(m_context, "xwalk"), XwalkGetter, NULL,
      External::New(m_context->GetIsolate(), this), DEFAULT, attributes);

  m_extensionObjectTemplate = ObjectTemplate::New(m_context->GetIsolate());
  m_extensionObjectTemplate->SetInternalFieldCount(1);

  std::lock_guard<std::mutex> guard(s_mutex);
  s_extensionManagerInstances[*m_context] = this;
  DEVICEAPI_LOG_INFO("ExtensionManagerInstance %zu => %zu",
                     s_extensionManagerInstances.size() - 1,
                     s_extensionManagerInstances.size());
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

  m_tizenValue.Reset();
  m_xwalkValue.Reset();
}

ExtensionManagerInstance* ExtensionManagerInstance::getExtensionInstance(
    Local<v8::Context> context) {
  std::lock_guard<std::mutex> guard(s_mutex);

  auto it = s_extensionManagerInstances.find(*context);
  if (it != s_extensionManagerInstances.end()) {
    return it->second;
  }
  DEVICEAPI_LOG_ERROR("Cannot find ExtensionManagerInstance!");
  return nullptr;
}

ExtensionManagerInstance* initialize(Local<Context> context) {
  DEVICEAPI_LOG_INFO("ExtensionManagerInstance Enter with context %p",
                     *context);
  return new ExtensionManagerInstance(context);
}

void close(Local<Context> context) {
  DEVICEAPI_LOG_INFO("ExtensionManagerInstance Enter with context %p",
                     *context);
  delete ExtensionManagerInstance::getExtensionInstance(context);
}
}  // namespace DeviceAPI
