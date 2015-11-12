#pragma once

#include <boost/optional.hpp>
#include <QAbstractTableModel>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "CommandEvent.h"
#include "Keymap.h"
#include "core/macros.h"

class KeymapTableModel;

class KeymapSortFilterProxyModel : public QSortFilterProxyModel {
  Q_OBJECT

 public:
  KeymapSortFilterProxyModel(KeymapTableModel* model, QObject* parent = 0);
  ~KeymapSortFilterProxyModel() = default;

  void setFilterText(const QString& text);

 protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

  QString m_filterText;
};

class KeymapTableModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  static constexpr int COMMAND_INDEX = 0;
  static constexpr int DESCRIPTION_INDEX = 1;
  static constexpr int KEY_INDEX = 2;
  static constexpr int IF_INDEX = 3;
  static constexpr int SOURCE_INDEX = 4;

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

  void setFilterText(const QString& text);

 protected:
  void contextMenuEvent(QContextMenuEvent* event) override;

 private:
  KeymapTableModel* m_model;
  QAction* m_copy;
  KeymapSortFilterProxyModel* m_proxyModel;
};
