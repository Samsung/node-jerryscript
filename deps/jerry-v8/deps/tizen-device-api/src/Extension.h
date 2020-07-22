// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

#ifndef WRT_SERVICE_NODE_EXTENSION_H_
#define WRT_SERVICE_NODE_EXTENSION_H_

#include <string>
#include <vector>

#include "v8.h"
#include "XW_Extension.h"
#include "XW_Extension_SyncMessage.h"
#include "XW_Extension_Data.h"

namespace wrt {
class RuntimeVariableProvider;

namespace xwalk {

class ExtensionAdapter;
class ExtensionInstance;

class Extension {
 public:
  class RuntimeVariableProvider {
   public:
    virtual void GetRuntimeVariable(const char* key, char* value,
                                    size_t value_len) = 0;
  };

  Extension(const std::string& path, RuntimeVariableProvider* provider);
  Extension(const std::string& path, const std::string& name,
            const std::vector<std::string>& entry_points,
            RuntimeVariableProvider* provider);

  virtual ~Extension();

  bool Initialize();
  ExtensionInstance* CreateInstance();

  XW_Extension xw_extension() { return xw_extension_; }

  std::string name() { return name_; }

  std::string javascript_api() {
    Initialize();
    return javascript_api_;
  }

  std::vector<std::string>& entry_points() { return entry_points_; }

  bool use_trampoline() { return use_trampoline_; }

  void set_name(const std::string& name) { name_ = name; }

  void set_javascript_api(const std::string& javascript_api) {
    javascript_api_ = javascript_api;
  }

  void set_use_trampoline(bool use_trampoline) {
    use_trampoline_ = use_trampoline;
  }

 private:
  friend class ExtensionAdapter;
  friend class ExtensionInstance;

  void GetRuntimeVariable(const char* key, char* value, size_t value_len);
  int CheckAPIAccessControl(const char* api_name);
  int RegisterPermissions(const char* perm_table);

  bool initialized_{false};
  void* handle_{nullptr};
  std::string library_path_;

  XW_Extension xw_extension_{0};
  std::string name_;
  std::string javascript_api_;
  std::vector<std::string> entry_points_;
  bool use_trampoline_{true};

  XW_CreatedInstanceCallback created_instance_callback_{nullptr};
  XW_DestroyedInstanceCallback destroyed_instance_callback_{nullptr};
  XW_ShutdownCallback shutdown_callback_{nullptr};
  XW_HandleMessageCallback handle_msg_callback_{nullptr};
  XW_HandleSyncMessageCallback handle_sync_msg_callback_{nullptr};
  XW_HandleDataCallback handle_data_callback_{nullptr};
  XW_HandleDataCallback handle_sync_data_callback_{nullptr};
  RuntimeVariableProvider* rv_provider_{nullptr};
};

class PostMessageListener {
 public:
  virtual void PostMessageToJS(const std::string& msg) = 0;
};

class PostDataListener {
 public:
  virtual void PostDataToJS(const std::string& msg, uint8_t* buffer,
                            size_t len) = 0;
};

class ExtensionInstance {
 public:
  ExtensionInstance(Extension* extension, XW_Instance xw_instance);
  virtual ~ExtensionInstance();

  void HandleMessage(const std::string& msg);
  void HandleSyncMessage(const std::string& msg);

  void HandleData(const std::string& msg, uint8_t* buffer, size_t len);
  void HandleSyncData(const std::string& msg, uint8_t* buffer, size_t len);

  XW_Instance xw_instance() { return xw_instance_; }

  std::string sync_replay_msg() { return sync_reply_msg_; }

  std::string sync_data_reply_msg(uint8_t** buffer, size_t* len) {
    *buffer = sync_reply_buffer_;
    *len = sync_reply_buffer_len_;
    return sync_reply_msg_;
  }

  void set_post_message_listener(PostMessageListener* listener) {
    post_message_listener_ = listener;
  }

  void set_post_data_listener(PostDataListener* listener) {
    post_data_listener_ = listener;
  }

 private:
  friend class ExtensionAdapter;

  void PostMessage(const std::string& msg);
  void SyncReply(const std::string& reply);

  void PostData(const std::string& msg, uint8_t* buffer, size_t len);
  void SyncDataReply(const std::string& reply, uint8_t* buffer, size_t len);

  Extension* extension_;
  XW_Instance xw_instance_;
  void* instance_data_;
  std::string sync_reply_msg_;
  uint8_t* sync_reply_buffer_;
  size_t sync_reply_buffer_len_;

  PostMessageListener* post_message_listener_;
  PostDataListener* post_data_listener_;
};

}  // namespace xwalk
}  // namespace wrt

namespace DeviceAPI {

class ESPostListener {
 public:
  virtual ~ESPostListener();
  void finalize();

 protected:
  ESPostListener(v8::Local<v8::Context> context,
                 v8::Local<v8::Object> listener);

  v8::Local<v8::Context> context_;
  v8::Persistent<v8::Object> listener_;
};

class ESPostMessageListener : public wrt::xwalk::PostMessageListener,
                              public ESPostListener {
 public:
  static ESPostMessageListener* create(v8::Local<v8::Context> context,
                                       v8::Local<v8::Object> listener) {
    return new ESPostMessageListener(context, listener);
  }
  void PostMessageToJS(const std::string& msg);

  typedef int (*Idler_t)(void* data);
  typedef void (*IdlerRegister_t)(Idler_t idler, void* data);
  static void SetMainThreadIdlerRegister(IdlerRegister_t idlerRegister) {
    AddIdlerToMainThread = idlerRegister;
  }

 private:
  ESPostMessageListener(v8::Local<v8::Context> context,
                        v8::Local<v8::Object> listener)
      : ESPostListener(context, listener) {}

  static IdlerRegister_t AddIdlerToMainThread;
};

class ESPostDataListener : public wrt::xwalk::PostDataListener,
                           public ESPostListener {
 public:
  static ESPostDataListener* create(v8::Local<v8::Context> context,
                                    v8::Local<v8::Object> listener) {
    return new ESPostDataListener(context, listener);
  }
  void PostDataToJS(const std::string& msg, uint8_t* buffer, size_t len);

 private:
  ESPostDataListener(v8::Local<v8::Context> context,
                     v8::Local<v8::Object> listener)
      : ESPostListener(context, listener) {}
};

}  // namespace DeviceAPI

#endif  // WRT_SERVICE_NODE_EXTENSION_H_
