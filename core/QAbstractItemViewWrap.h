#pragma once

#include <QAbstractItemView>

#include "Wrapper.h"

namespace core {

class QAbstractItemViewWrap : public Wrapper {
  Q_OBJECT
  Q_CLASSINFO(WRAPPED, "QAbstractItemView*")

 public:
  QAbstractItemViewWrap(QAbstractItemView* event) { m_wrapped = QVariant::fromValue(event); }

  ~QAbstractItemViewWrap() = default;

 public slots:
  int sizeHintForRow(int row) const;
  int sizeHintForColumn(int column) const;
  void setCurrentIndex(const QModelIndex& index);
  QScrollBar* verticalScrollBar() const;
  bool isVisible() const;
  void hide();

  void setSelectionModel(QItemSelectionModel* selectionModel);
  QItemSelectionModel* selectionModel() const;

 private:
};

}  // namespace core

Q_DECLARE_METATYPE(core::QAbstractItemViewWrap*)
Q_DECLARE_METATYPE(QAbstractItemView*)
