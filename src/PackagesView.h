#pragma once

#include <unordered_map>
#include <boost/optional.hpp>
#include <QNetworkAccessManager>
#include <QWidget>
#include <QLabel>
#include <QJsonObject>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QMap>
#include <QMovie>

#include "core/macros.h"
#include "core/Package.h"

namespace Ui {
class PackagesView;
}

class PackageTableModel;
class PackageDelegate;
class QFile;

class PackagesView : public QWidget {
  Q_OBJECT

 public:
  explicit PackagesView(QWidget* parent = 0);
  ~PackagesView();

  void startLoading();

signals:
  void installationFailed(const QModelIndex& index);
  void installationSucceeded(const QModelIndex& index);

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
  QNetworkReply* sendGetRequest(const QString& url);
  QNetworkReply* sendGetRequest(const QUrl& url);
  void startDownloadingPackage(const QModelIndex& index);
  void installPackage(QNetworkReply* reply, const QModelIndex& index, const core::Package& pkg);
};

class PackageDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  enum ButtonState { Raised, Pressed, Installing, Installed };
  explicit PackageDelegate(QObject* parent = nullptr);

  void setMovie(int row, std::unique_ptr<QMovie> movie);
  void stopMovie(int row);

signals:
  void needsUpdate(const QModelIndex& index);
  void clicked(const QModelIndex& index);

 protected:
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  bool editorEvent(QEvent* event,
                   QAbstractItemModel* model,
                   const QStyleOptionViewItem& option,
                   const QModelIndex& index) override;

 private:
  std::unordered_map<int, std::unique_ptr<QMovie>> m_rowMovieMap;
  void initButtonStyleOption(const QModelIndex& index,
                             const QStyleOptionViewItem& option,
                             QStyleOptionButton* btnOption) const;
  bool hitTestWithButton(QEvent* event,
                         const QModelIndex& index,
                         const QStyleOptionViewItem& option);
};

// class with Q_OBJECT macro must be declared in a header.
class PackageTableModel : public QAbstractTableModel {
  Q_OBJECT

 public:
  static const int BUTTON_COLUMN = 0;

  explicit PackageTableModel(QObject* parent = 0);

  void setPackages(const QList<core::Package>& packages);
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  boost::optional<core::Package> package(int row);

signals:
  void clicked(const core::Package& package);

 private:
  QList<core::Package> m_packages;
  QMap<QModelIndex, PackageDelegate::ButtonState> m_buttonStateMap;
};
