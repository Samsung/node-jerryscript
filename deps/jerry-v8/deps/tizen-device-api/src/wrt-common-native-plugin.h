// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WRT_COMMON_NATIVE_PLUGIN_H_
#define WRT_COMMON_NATIVE_PLUGIN_H_

#include <string>

namespace wrt {
namespace common {

class NativePlugin {
 public:
  virtual void OnLoad() = 0;
  virtual std::string OnCallSync(std::string& data) = 0;
  virtual std::string OnCall(std::string& data, int callback_handle) = 0;
};

typedef NativePlugin* create_native_plugin_t();

}  // namespace common
}  // namespace wrt

#define EXPORT_NATIVE_PLUGIN(pluginClass)                        \
  extern "C" wrt::common::NativePlugin* create_native_plugin() { \
    return (wrt::common::NativePlugin*)(new pluginClass());      \
  }

#endif  // WRT_COMMON_NATIVE_PLUGIN_H_
