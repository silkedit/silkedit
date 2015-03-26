#pragma once

#include <msgpack.hpp>
#include <memory>
#include "macros.h"

struct object_with_zone {
  DEFAULT_COPY_AND_MOVE(object_with_zone)

  object_with_zone() = default;
  object_with_zone(const msgpack::object& obj, std::shared_ptr<msgpack::zone> z)
      : object(obj), zone(z) {}

  ~object_with_zone() = default;

  msgpack::object object;
  std::shared_ptr<msgpack::zone> zone;
};

Q_DECLARE_METATYPE(object_with_zone);
