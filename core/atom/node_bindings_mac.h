// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_COMMON_NODE_BINDINGS_MAC_H_
#define ATOM_COMMON_NODE_BINDINGS_MAC_H_

#include "node_bindings.h"
#include "core/macros.h"

namespace atom {

class NodeBindingsMac : public NodeBindings {
  DISABLE_COPY(NodeBindingsMac)
 public:
  NodeBindingsMac();
  virtual ~NodeBindingsMac();

  void RunMessageLoop() override;

 private:
  // Called when uv's watcher queue changes.
  static void OnWatcherQueueChanged(uv_loop_t* loop);

  void PollEvents() override;
};

}  // namespace atom

#endif  // ATOM_COMMON_NODE_BINDINGS_MAC_H_
