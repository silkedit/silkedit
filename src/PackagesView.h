#pragma once

#include <QNetworkAccessManager>
#include <QWidget>
#include <QLabel>
#include <QJsonObject>
#include <QAbstractTableModel>

#include "core/macros.h"

namespace Ui {
class PackagesView;
}

class QProgressIndicator;
class PackageTableModel;

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

class PackageTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  explicit PackageTableModel(QObject* parent = 0);

  void setPackages(const QList<Package>& packages);
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  QList<Package> m_packages;
};
