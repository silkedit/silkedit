#pragma once

#include <unordered_map>
#include <boost/optional.hpp>
#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QJsonObject>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QMap>
#include <QMovie>
#include <QSortFilterProxyModel>

#include "core/macros.h"
#include "core/Package.h"

namespace Ui {
class PackagesView;
}

class PackageTableModel;
class PackageDelegate;
class QFile;

class PackagesViewModel : public QObject {
  Q_OBJECT
 public:
  PackagesViewModel(QObject* parent);
  virtual void loadPackages() = 0;
  virtual QString buttonText() = 0;
  virtual QString TextAfterProcess() = 0;
  virtual void processWithPackage(const QModelIndex& index, const core::Package& pkg) = 0;

signals:
  void packagesLoaded(QList<core::Package> packages);
  void processFailed(const QModelIndex& index);
  void processSucceeded(const QModelIndex& index);

 protected:
  QSet<core::Package> installedPackages();
};

class AvailablePackagesViewModel : public PackagesViewModel {
  Q_OBJECT
 public:
  AvailablePackagesViewModel(QObject* parent);
  // load packages asynchronously
  void loadPackages() override;
  QString buttonText() override;
  QString TextAfterProcess() override;
  void processWithPackage(const QModelIndex& index, const core::Package& pkg) override;
};

class InstalledPackagesViewModel : public PackagesViewModel {
  Q_OBJECT
 public:
  InstalledPackagesViewModel(QObject* parent);
  void loadPackages() override;
  QString buttonText() override;
  QString TextAfterProcess() override;
  void processWithPackage(const QModelIndex& index, const core::Package& pkg) override;
};

class PackagesView : public QWidget {
  Q_OBJECT

 public:
  PackagesView(PackagesViewModel* viewModel, QWidget* parent = 0);
  ~PackagesView();

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  Ui::PackagesView* ui;
  PackageTableModel* m_pkgsModel;
  PackageDelegate* m_delegate;
  PackagesViewModel* m_viewModel;
  QSortFilterProxyModel* m_proxyModel;

  void startLoading();
  void startAnimation();
  void stopAnimation();
  void processWithPackage(const QModelIndex& index);
  void onProcessFailed(const QModelIndex& index);
  void onProcessSucceeded(const QModelIndex& index);
};

class PackageDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  enum ButtonState { Raised, Pressed, Installing, Installed };
  explicit PackageDelegate(const QString& buttonText,
                           const QString& textAfterProcess,
                           QObject* parent = nullptr);

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
  QString m_buttonText;
  QString m_textAfterProcess;

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
