#pragma once

#include <boost/optional.hpp>
#include <QAbstractTableModel>
#include <QTableView>

#include "CommandEvent.h"
#include "Keymap.h"
#include "core/macros.h"

class KeymapTableModel : public QAbstractTableModel {
 public:
  KeymapTableModel(QObject* parent = 0);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  boost::optional<Keymap> keymapAt(int row);

 private:
  QList<Keymap> m_keymaps;
  void init();
};

class KeymapTableView : public QTableView {
  Q_OBJECT
  DISABLE_COPY(KeymapTableView)

 public:
  KeymapTableView(QWidget* parent = 0);
  ~KeymapTableView() = default;
  DEFAULT_MOVE(KeymapTableView)

 protected:
  void contextMenuEvent(QContextMenuEvent* event) override;

 private:
  KeymapTableModel* m_model;
  QAction* m_copy;
};
