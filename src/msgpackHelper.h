#pragma once

#include <msgpack.hpp>
#include <memory>
#include "core/macros.h"

struct object_with_zone {
  DEFAULT_MOVE(object_with_zone)
  DISABLE_COPY(object_with_zone)

  object_with_zone(const msgpack::object& obj, std::unique_ptr<msgpack::zone> z)
      : object(obj), zone(std::move(z)) {}

  ~object_with_zone() = default;

  msgpack::object object;
  std::unique_ptr<msgpack::zone> zone;
};
