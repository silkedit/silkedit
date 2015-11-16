#pragma once

#include <msgpack/versioning.hpp>
#include <msgpack/adaptor/adaptor_base.hpp>
#include <msgpack/adaptor/check_container_size.hpp>

#include <QString>

namespace msgpack {

/// @cond
MSGPACK_API_VERSION_NAMESPACE(v1) {
  /// @endcond

  namespace adaptor {

  template <>
  struct convert<QString> {
    msgpack::object const& operator()(msgpack::object const& o, QString& v) const {
      switch (o.type) {
        case msgpack::type::BIN:
          v = std::move(QString::fromUtf8(o.via.bin.ptr, o.via.bin.size));
          break;
        case msgpack::type::STR:
          v = std::move(QString::fromUtf8(o.via.str.ptr, o.via.str.size));
          break;
        default:
          throw msgpack::type_error();
          break;
      }
      return o;
    }
  };

  template <>
  struct pack<QString> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const QString& v) const {
      uint32_t size = checked_get_container_size(v.size());
      o.pack_str(size);
      o.pack_str_body(v.toUtf8().constData(), size);
      return o;
    }
  };

  template <>
  struct object<QString> {
    void operator()(msgpack::object& o, const QString& v) const {
      uint32_t size = checked_get_container_size(v.size());
      o.type = msgpack::type::STR;
      o.via.str.ptr = v.toUtf8().constData();
      o.via.str.size = size;
    }
  };

  template <>
  struct object_with_zone<QString> {
    void operator()(msgpack::object::with_zone& o, const QString& v) const {
      uint32_t size = checked_get_container_size(v.size());
      o.type = msgpack::type::STR;
      char* ptr = static_cast<char*>(o.zone.allocate_align(size));
      o.via.str.ptr = ptr;
      o.via.str.size = size;
      std::memcpy(ptr, v.toUtf8().constData(), v.size());
    }
  };

  }  // namespace adaptor

  /// @cond
}  // MSGPACK_API_VERSION_NAMESPACE(v1)
/// @endcond

}  // namespace msgpack
