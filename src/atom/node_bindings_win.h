// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_COMMON_NODE_BINDINGS_WIN_H_
#define ATOM_COMMON_NODE_BINDINGS_WIN_H_

#include "node_bindings.h"
#include "core/macros.h"

namespace atom {

class NodeBindingsWin : public NodeBindings {
  DISABLE_COPY(NodeBindingsWin)
 public:
  NodeBindingsWin();
  virtual ~NodeBindingsWin();

 private:
  void PollEvents() override;
};

}  // namespace atom

#endif  // ATOM_COMMON_NODE_BINDINGS_WIN_H_
