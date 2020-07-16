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

#ifndef NODE_BINDINGS_H
#define NODE_BINDINGS_H

#include <functional>

namespace v8 {
class Isolate;
}

namespace node {
class Environment;
}

typedef struct uv_loop_s uv_loop_t;

namespace nodejerry {

class NodeBindings {
 public:
  NodeBindings();
  virtual ~NodeBindings(){};

  struct Platform {
    void (*DrainVMTasks)(v8::Isolate* isolate);
  };

  struct Environment {
    std::function<v8::Isolate*()> isolate;
    std::function<uv_loop_t*()> event_loop;
  };

  struct Node {
    std::function<void()> EmitBeforeExit;
  };

  void Initialize(Environment&& env, Platform&& platform, Node&& node);
  void StartEventLoop();
  void RunOnce();
  bool HasMoreTasks();

 private:
  Environment m_env;
  Platform m_platform;
  Node m_node;
  bool m_isInitialize = {false};
  bool m_hasMoreNodeTasks = {true};
};

}  // namespace nodejerry

#endif
