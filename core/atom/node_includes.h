// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_COMMON_NODE_INCLUDES_H_
#define ATOM_COMMON_NODE_INCLUDES_H_

#ifdef __clang__
#pragma clang system_header
#endif

// Include common headers for using node APIs.

#define BUILDING_NODE_EXTENSION

#undef ASSERT
#undef CHECK
#undef CHECK_EQ
#undef CHECK_NE
#undef CHECK_GE
#undef CHECK_GT
#undef CHECK_LE
#undef CHECK_LT
#undef DISALLOW_COPY_AND_ASSIGN
#undef NO_RETURN
#undef debug_string  // This is defined in OS X 10.9 SDK in AssertMacros.h.
// Suppress MSVC warnings
#ifdef _WIN32
#pragma warning(push, 0)
#endif
#include <env.h>
#include <env-inl.h>
#include <node.h>
#include <node_buffer.h>
#include <node_internals.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#endif  // ATOM_COMMON_NODE_INCLUDES_H_
