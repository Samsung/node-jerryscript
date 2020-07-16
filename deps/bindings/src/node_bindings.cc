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

#include <glib.h>
#include <uv.h>
#include <cassert>
#include "node_bindings.h"

namespace glib {

using namespace nodejerry;

struct SourceData {
  GSource source;
  gpointer tag;
  NodeBindings* node_bindings;
};

// NOTE: classify EventLoop for other methods if needed.

static GMainContext* gcontext;
static GMainLoop* gmainLoop;
static GSource* uvsource;
static GSourceFuncs source_funcs;
static bool gmainLoopDone = false;

static gboolean GmainLoopPrepareCallback(GSource* source, gint* timeout) {
  uv_update_time(uv_default_loop());
  *timeout = uv_backend_timeout(uv_default_loop());

  if (!uv_watcher_queue_empty(uv_default_loop())) {
    return TRUE;
  }

  return 0 == *timeout;
}

static gboolean GmainLoopCheckCallback(GSource* source) {
  if (!uv_backend_timeout(uv_default_loop())) {
    return TRUE;
  }

  return (G_IO_IN ==
          g_source_query_unix_fd(source, ((SourceData*)source)->tag));
}

static gboolean GmainLoopDispatchCallback(GSource* source, GSourceFunc callback,
                                          gpointer user_data) {
  assert(gcontext);
  g_main_context_iteration(gcontext, FALSE);

  if (gmainLoopDone) {
    return G_SOURCE_REMOVE;
  }

  NodeBindings* node_bindings = ((SourceData*)source)->node_bindings;

  node_bindings->RunOnce();

  if (!node_bindings->HasMoreTasks()) {
    g_main_loop_quit(gmainLoop);
    return G_SOURCE_REMOVE;
  }
  return G_SOURCE_CONTINUE;
}

static void GmainLoopInit(NodeBindings* self) {
  gcontext = g_main_context_default();
  gmainLoop = g_main_loop_new(gcontext, FALSE);
  source_funcs = {
      .prepare = GmainLoopPrepareCallback,
      .check = GmainLoopCheckCallback,
      .dispatch = GmainLoopDispatchCallback,
  };

  uvsource = g_source_new(&source_funcs, sizeof(SourceData));
  ((SourceData*)uvsource)->tag = g_source_add_unix_fd(
      uvsource, uv_backend_fd(uv_default_loop()),
      (GIOCondition)(G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_PRI));
  ((SourceData*)uvsource)->node_bindings = self;

  g_source_attach(uvsource, gcontext);
  g_source_unref(uvsource);
}

void GmainLoopStart() {
  assert(gmainLoop);
  assert(gcontext);

  g_main_loop_run(gmainLoop);
  gmainLoopDone = true;
  g_main_context_iteration(gcontext, TRUE);
}

void GmainLoopExit() {
  if (uvsource) {
    g_source_destroy(uvsource);
  }
  if (gmainLoop) {
    g_main_loop_unref(gmainLoop);
  }
  if(gcontext) {
    g_main_context_unref(gcontext);
  }
}

}  // namespace glib

namespace nodejerry {

NodeBindings::NodeBindings() {}

void NodeBindings::Initialize(Environment&& env, Platform&& platform,
                              Node&& node) {
  assert(platform.DrainVMTasks);

  m_env = std::move(env);
  m_platform = std::move(platform);
  m_node = std::move(node);
  m_isInitialize = true;
}

void NodeBindings::StartEventLoop() {
  assert(m_isInitialize);

  glib::GmainLoopInit(this);

  RunOnce();

  if (HasMoreTasks()) {
    glib::GmainLoopStart();
  }

  glib::GmainLoopExit();
}

bool NodeBindings::HasMoreTasks() { return m_hasMoreNodeTasks; }

void NodeBindings::RunOnce() {
  auto isolate = m_env.isolate();
  auto event_loop = m_env.event_loop();
  bool more;
  
  uv_run(event_loop, UV_RUN_NOWAIT);
  
  m_platform.DrainVMTasks(isolate);

  more = uv_loop_alive(event_loop);

  if (more == false) {
    m_node.EmitBeforeExit();

    // Emit `beforeExit` if the loop became alive either after emitting
    // event, or after running some callbacks.
    more = uv_loop_alive(event_loop);
  }
  m_hasMoreNodeTasks = more;
}

}  // namespace nodejerry
