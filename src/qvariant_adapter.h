#pragma once

#include <boost/optional.hpp>
#include <msgpack/versioning.hpp>
#include <QVariant>

#include "StatusBar.h"
#include "TextEditView.h"
#include "TabView.h"
#include "TabViewGroup.h"
#include "Window.h"
#include "InputDialog.h"
#include "core/qstring_adapter.h"

// User defined class template specialization
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
  namespace adaptor {

  template <>
  struct pack<QVariant> {
    template <typename Stream>
    packer<Stream>& operator()(msgpack::packer<Stream>& o, QVariant const& v) const {
      // todo: is there better way?
      if (v.canConvert<StatusBar*>()) {
        o.pack(v.value<StatusBar*>()->id());
        return o;
      } else if (v.canConvert<TextEditView*>()) {
        o.pack(v.value<TextEditView*>()->id());
        return o;
      } else if (v.canConvert<TabView*>()) {
        o.pack(v.value<TabView*>()->id());
        return o;
      } else if (v.canConvert<TabViewGroup*>()) {
        o.pack(v.value<TabViewGroup*>()->id());
        return o;
      } else if (v.canConvert<Window*>()) {
        o.pack(v.value<Window*>()->id());
        return o;
      } else if (v.canConvert<InputDialog*>()) {
        o.pack(v.value<InputDialog*>()->id());
        return o;
      } else if (v.canConvert<boost::optional<QString>>()) {
        const auto& strOpt = v.value<boost::optional<QString>>();
        if (strOpt) {
          o.pack<QString>(*strOpt);
        } else {
          o.pack_nil();
        }
        return o;
      } else if (v.canConvert<QList<Window*>>()) {
        QList<Window*> windows = v.value<QList<Window*>>();
        uint32_t size = checked_get_container_size(windows.size());
        o.pack_array(size);
        for (Window* win : windows) {
          o.pack_int(win->id());
        }
        return o;
      }

      switch (v.type()) {
        case QVariant::Bool:
          o.pack(v.toBool());
          break;
        case QVariant::Int:
          o.pack(v.toInt());
          break;
        case QVariant::UInt:
          o.pack(v.toUInt());
          break;
        case QVariant::LongLong:
          o.pack(v.toLongLong());
          break;
        case QVariant::ULongLong:
          o.pack(v.toULongLong());
          break;
        case QVariant::Double:
          o.pack(v.toDouble());
          break;
        case QVariant::ByteArray:
          o.pack(v.toByteArray().constData());
          break;
        case QVariant::String:
          o.pack<QString>(v.toString());
          break;
        case QVariant::StringList: {
          const QStringList& strList = v.toStringList();
          uint32_t size = checked_get_container_size(strList.size());
          o.pack_array(size);
          for (const QString& str : strList) {
            o.pack<QString>(str);
          }
          break;
        }
        default:
          qWarning("type %s not supported", qPrintable(v.typeName()));
          o.pack_nil();
          break;
      }
      return o;
    }
  };

  }  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
}  // namespace msgpack
