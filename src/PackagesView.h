#pragma once

#include <QNetworkAccessManager>
#include <QWidget>
#include <QLabel>
#include <QJsonObject>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QMap>

#include "core/macros.h"

namespace Ui {
class PackagesView;
}

class QProgressIndicator;
class PackageTableModel;
class PackageDelegate;

class PackagesView : public QWidget {
  Q_OBJECT

 public:
  explicit PackagesView(QWidget* parent = 0);
  ~PackagesView();

  void startLoading();

 protected:
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

 private:
  Ui::PackagesView* ui;
  QNetworkAccessManager* m_accessManager;
  QNetworkReply* m_reply;
  PackageTableModel* m_pkgsModel;
  PackageDelegate* m_delegate;

  void handleError(QNetworkReply* reply);
  void startAnimation();
  void stopAnimation();
};

// Package model class
struct Package {
  DEFAULT_COPY_AND_MOVE(Package)

  static const int ITEM_COUNT = 4;
  static Package fromJson(const QJsonValue& value) { return std::move(Package(value)); }

  QString name;
  QString version;
  QString description;
  QString repository;

  explicit Package(const QJsonValue& jsonValue);
  ~Package() = default;
};

class PackageDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  enum ButtonState { Raised = 100, Pressed = 200 };
  explicit PackageDelegate(QObject* parent = nullptr);

signals:
  void needsUpdate(const QModelIndex& index);

 protected:
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  bool editorEvent(QEvent* event,
                   QAbstractItemModel* model,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

 private:
  void initButtonStyleOption(const QModelIndex& index,
                             const QStyleOptionViewItem& option,
                             QStyleOptionButton* btnOption) const;
  bool hitTestWithbutton(QEvent* event,
                         const QModelIndex& index,
                         const QStyleOptionViewItem& option);
};

// class with Q_OBJECT macro must be declared in a header.
class PackageTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  static const int BUTTON_COLUMN = 0;
  explicit PackageTableModel(QObject* parent = 0);

  void setPackages(const QList<Package>& packages);
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  QList<Package> m_packages;
  QMap<QModelIndex, PackageDelegate::ButtonState> m_buttonStateMap;
};
