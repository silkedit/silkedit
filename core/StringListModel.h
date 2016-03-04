#pragma once

#include <QStringListModel>

#include "core/macros.h"

namespace core {

class StringListModel : public QStringListModel {
  Q_OBJECT
  DISABLE_COPY(StringListModel)

 public:
  Q_INVOKABLE StringListModel(QObject* parent = 0);
  ~StringListModel() = default;
  DEFAULT_MOVE(StringListModel)

 public slots:
  QStringList stringList() const;
  void setStringList(const QVariantList& strings);

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::StringListModel*)
