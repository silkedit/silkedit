#pragma once

#include <msgpack/versioning.hpp>
#include <msgpack/adaptor/adaptor_base.hpp>
#include <msgpack/adaptor/check_container_size.hpp>
#include <QUuid>
#include <QDebug>

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
  /// @endcond

  namespace adaptor {

  template <>
  struct convert<QUuid> {
    msgpack::object const& operator()(msgpack::object const& o, QUuid& v) const {
      switch (o.type) {
        case msgpack::type::BIN:
          v = std::move(QUuid(QByteArray(o.via.bin.ptr, o.via.bin.size)));
          break;
        default:
          throw msgpack::type_error();
          break;
      }
      return o;
    }
  };

  template <>
  struct pack<QUuid> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const QUuid& v) const {
      QByteArray bytes = v.toByteArray();
      uint32_t size = checked_get_container_size(bytes.size());
      o.pack_bin(size);
      o.pack_bin_body(bytes.constData(), size);
      return o;
    }
  };

  template <>
  struct object<QUuid> {
    void operator()(msgpack::object& o, const QUuid& v) const {
      QByteArray bytes = v.toByteArray();
      uint32_t size = checked_get_container_size(bytes.size());
      o.type = msgpack::type::BIN;
      o.via.bin.ptr = bytes.constData();
      o.via.bin.size = size;
    }
  };

  template <>
  struct object_with_zone<QUuid> {
    void operator()(msgpack::object::with_zone& o, const QUuid& v) const {
      QByteArray bytes = v.toByteArray();
      uint32_t size = checked_get_container_size(bytes.size());
      o.type = msgpack::type::BIN;
      char* ptr = static_cast<char*>(o.zone.allocate_align(size));
      o.via.bin.ptr = ptr;
      o.via.bin.size = size;
      std::memcpy(ptr, bytes.constData(), bytes.size());
    }
  };

  }  // namespace adaptor

  /// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace msgpack
