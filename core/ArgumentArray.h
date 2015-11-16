#pragma once

#include <msgpack/versioning.hpp>
#include <QVariantList>

#include "qstring_adapter.h"

namespace {
QVariant toVariant(const msgpack::object& obj) {
  switch (obj.type) {
    case msgpack::type::object_type::BOOLEAN:
      return QVariant::fromValue(obj.as<bool>());
    case msgpack::type::object_type::POSITIVE_INTEGER:
    case msgpack::type::object_type::NEGATIVE_INTEGER:
      return QVariant::fromValue(obj.as<int>());
    case msgpack::type::object_type::FLOAT:
      return QVariant::fromValue(obj.as<double>());
    case msgpack::type::object_type::STR:
      return QVariant::fromValue(obj.as<QString>());
    case msgpack::type::object_type::ARRAY: {
      QVariantList list;
      for (uint32_t i = 0; i < obj.via.array.size; i++) {
        list.append(toVariant(obj.via.array.ptr[i]));
      }
      return list;
    }
    default:
      return QVariant();
  }
}
}

namespace core {

class ArgumentArray {
 public:
  // When you want to convert from msgpack::object to ArgumentArray, ArgumentArray should be default
  // constractible.
  ArgumentArray() {}
  ArgumentArray(int id, QVariantList args) : m_id(id), m_args(args) {}
  int id() { return m_id; }
  QVariantList args() { return m_args; }
  int size() { return 1 + m_args.size(); }

 private:
  int m_id;
  QVariantList m_args;
};

}  // namespace core

// User defined class template specialization
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
  namespace adaptor {

  template <>
  struct convert<core::ArgumentArray> {
    msgpack::object const& operator()(msgpack::object const& o, core::ArgumentArray& v) const {
      QVariantList varArgs;
      if (o.type != msgpack::type::ARRAY || o.via.array.size == 0 ||
          !(o.via.array.ptr[0].type == msgpack::type::POSITIVE_INTEGER ||
            o.via.array.ptr[0].type == msgpack::type::NEGATIVE_INTEGER)) {
        throw msgpack::type_error();
      }
      int id = o.via.array.ptr[0].as<int>();
      for (uint32_t i = 1; i < o.via.array.size; i++) {
        varArgs.append(toVariant(o.via.array.ptr[i]));
      }
      v = core::ArgumentArray(id, varArgs);
      return o;
    }
  };

  }  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
}  // namespace msgpack
