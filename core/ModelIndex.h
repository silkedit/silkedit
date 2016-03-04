#pragma once

#include <QModelIndex>
#include <QMetaType>

#include "Wrapper.h"

namespace core {

class ModelIndex : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QModelIndex")

 public:
  Q_INVOKABLE ModelIndex();
  ModelIndex(QModelIndex index);
  ~ModelIndex() = default;
};

}  // namespace core

Q_DECLARE_METATYPE(core::ModelIndex*)
Q_DECLARE_METATYPE(QModelIndex)
