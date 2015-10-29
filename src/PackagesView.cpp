#include <algorithm>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QAbstractTableModel>
#include <QTableView>
#include <QItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QDir>
#include <QProcess>
#include <QTimer>

#include "PackagesView.h"
#include "ui_PackagesView.h"
#include "PluginManager.h"
#include "SilkApp.h"
#include "core/Constants.h"
#include "core/scoped_guard.h"
#include "core/Util.h"

using core::Package;
using core::Constants;
using core::scoped_guard;
using core::Util;

PackagesView::PackagesView(PackagesViewModel* viewModel, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::PackagesView),
      m_pkgsModel(new PackageTableModel(this)),
      m_delegate(new PackageDelegate(viewModel->buttonText(), viewModel->TextAfterProcess(), this)),
      m_viewModel(viewModel) {
  ui->setupUi(this);
  QMovie* indicatorMovie = new QMovie(":/images/indicator.gif", QByteArray(), this);
  ui->indicatorLabel->setMovie(indicatorMovie);
  ui->indicatorLabel->hide();
  ui->tableView->setModel(m_pkgsModel);
  ui->tableView->setItemDelegate(m_delegate);
  connect(m_viewModel, &PackagesViewModel::packagesLoaded, [=](QList<Package> packages) {
    stopAnimation();
    ui->tableView->show();
    m_pkgsModel->setPackages(packages);
  });
  connect(m_viewModel, &PackagesViewModel::processFailed, this, &PackagesView::onProcessFailed);
  connect(m_viewModel, &PackagesViewModel::processSucceeded, this,
          &PackagesView::onProcessSucceeded);
  connect(m_delegate, &PackageDelegate::needsUpdate,
          [=](const QModelIndex& index) { ui->tableView->update(index); });
  connect(m_delegate, &PackageDelegate::clicked, this, &PackagesView::processWithPackage);
  setLayout(ui->rootHLayout);
}

PackagesView::~PackagesView() {
  qDebug("~PackagesView");
  // disconnect this manually because processFailed may be emitted in the destructor chain of
  // ~PackagesView()
  disconnect(m_viewModel, &PackagesViewModel::processFailed, this, &PackagesView::onProcessFailed);
  delete ui;
}

void PackagesView::startLoading() {
  ui->tableView->hide();
  startAnimation();
  m_viewModel->loadPackages();
}

void PackagesView::showEvent(QShowEvent*) {
  startLoading();
}

void PackagesView::startAnimation() {
  ui->indicatorLabel->show();
  ui->indicatorLabel->movie()->start();
}

void PackagesView::stopAnimation() {
  ui->indicatorLabel->hide();
  ui->indicatorLabel->movie()->stop();
}

void PackagesView::processWithPackage(const QModelIndex& index) {
  auto pkgOpt = m_pkgsModel->package(index.row());
  if (!pkgOpt) {
    qWarning("package not found. row: %d", index.row());
    emit m_viewModel->processFailed(index);
    return;
  }

  std::unique_ptr<QMovie> indicatorMovie(new QMovie(":/images/indicator.gif"));
  connect(indicatorMovie.get(), &QMovie::updated,
          [=](const QRect&) { ui->tableView->update(index); });
  indicatorMovie->start();
  m_delegate->setMovie(index.row(), std::move(indicatorMovie));

  // validate package
  Package pkg = *pkgOpt;
  QStringList validationErrors = pkg.validate();
  if (!validationErrors.isEmpty()) {
    for (const QString& msg : validationErrors) {
      qWarning() << msg;
    }
    emit m_viewModel->processFailed(index);
    return;
  }

  m_viewModel->processWithPackage(index, pkg);
}

void PackagesView::onProcessFailed(const QModelIndex& index) {
  m_pkgsModel->setData(index, (int)PackageDelegate::Raised, Qt::UserRole);
  m_delegate->stopMovie(index.row());
  ui->tableView->update(index);
}

void PackagesView::onProcessSucceeded(const QModelIndex& index) {
  m_delegate->stopMovie(index.row());
  m_pkgsModel->setData(index, (int)PackageDelegate::Installed, Qt::UserRole);
  ui->tableView->update(index);
}

PackageTableModel::PackageTableModel(QObject* parent) : QAbstractTableModel(parent) {
}

void PackageTableModel::setPackages(const QList<Package>& packages) {
  beginResetModel();
  m_packages = packages;
  m_buttonStateMap.clear();
  endResetModel();
}

int PackageTableModel::rowCount(const QModelIndex&) const {
  return m_packages.size();
}

int PackageTableModel::columnCount(const QModelIndex&) const {
  return 4;
}

QVariant PackageTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (role == Qt::UserRole) {
    if (m_buttonStateMap.contains(index)) {
      return QVariant(m_buttonStateMap[index]);
    }

    return QVariant();
  }

  if (role != Qt::DisplayRole || index.row() >= m_packages.size() ||
      index.column() >= Package::ITEM_COUNT) {
    return QVariant();
  }

  const Package& pkg = m_packages[index.row()];
  switch (index.column()) {
    case 1:
      return pkg.name;
    case 2:
      return pkg.version;
    case 3:
      return pkg.description;
    default:
      return QVariant();
  }
}

QVariant PackageTableModel::headerData(int, Qt::Orientation, int role) const {
  if (role == Qt::SizeHintRole)
    return QSize(1, 1);
  return QVariant();
}

boost::optional<Package> PackageTableModel::package(int row) {
  if (row < m_packages.size()) {
    return m_packages.at(row);
  } else {
    return boost::none;
  }
}

bool PackageTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::UserRole) {
    m_buttonStateMap[index] = (PackageDelegate::ButtonState)value.toInt();
    return true;
  } else {
    return QAbstractTableModel::setData(index, value, role);
  }
}

PackageDelegate::PackageDelegate(const QString& buttonText,
                                 const QString& textAfterProcess,
                                 QObject* parent)
    : QStyledItemDelegate(parent), m_buttonText(buttonText), m_textAfterProcess(textAfterProcess) {
}

void PackageDelegate::setMovie(int row, std::unique_ptr<QMovie> movie) {
  m_rowMovieMap[row] = std::move(movie);
}

void PackageDelegate::stopMovie(int row) {
  if (m_rowMovieMap.count(row) != 0) {
    m_rowMovieMap.erase(row);
  }
}

void PackageDelegate::paint(QPainter* painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const {
  if (!index.isValid() || index.column() != PackageTableModel::BUTTON_COLUMN) {
    return QStyledItemDelegate::paint(painter, option, index);
  }

  ButtonState s = (ButtonState)(index.data(Qt::UserRole).toInt());
  switch (s) {
    case Raised:
    case Pressed: {
      QStyleOptionButton opt;
      initButtonStyleOption(index, option, &opt);
      QApplication::style()->drawControl(QStyle::CE_PushButton, &opt, painter, 0);
      break;
    }
    case Installing: {
      // draw indicator
      if (m_rowMovieMap.count(index.row()) == 0) {
        qWarning("movie not found for row: %d", index.row());
        return;
      }

      int align = QStyle::visualAlignment(Qt::LeftToRight, Qt::AlignHCenter | Qt::AlignVCenter);
      QApplication::style()->drawItemPixmap(painter, option.rect, align,
                                            m_rowMovieMap.at(index.row())->currentPixmap());
      break;
    }
    case Installed: {
      int align = QStyle::visualAlignment(Qt::LeftToRight, Qt::AlignHCenter | Qt::AlignVCenter);
      QApplication::style()->drawItemText(painter, option.rect, align, option.palette, true,
                                          m_textAfterProcess, QPalette::WindowText);
      break;
    }
    default:
      qWarning("invalid ButtonState");
      break;
  }
}

bool PackageDelegate::hitTestWithButton(QEvent* event,
                                        const QModelIndex& index,
                                        const QStyleOptionViewItem& option) {
  QStyleOptionButton opt;
  initButtonStyleOption(index, option, &opt);
  QString text = tr("Install");
  QSize textSize = opt.fontMetrics.size(Qt::TextShowMnemonic, text);
  // fixme: btnSize is not correct.
  QSize btnSize =
      (QApplication::style()->sizeFromContents(QStyle::CT_PushButton, &opt, textSize, nullptr))
          .expandedTo(QApplication::globalStrut()) +
      QSize(10, -10);
  int wmargin = (option.rect.width() - btnSize.width()) / 2;
  int hmargin = (option.rect.height() - btnSize.height()) / 2;
  QRect buttonRect(QPoint(option.rect.left() + wmargin, option.rect.top() + hmargin), btnSize);
  QMouseEvent* mouseEv = static_cast<QMouseEvent*>(event);
  return buttonRect.contains(mouseEv->x(), mouseEv->y());
}

bool PackageDelegate::editorEvent(QEvent* event,
                                  QAbstractItemModel* model,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) {
  if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
    bool hit = hitTestWithButton(event, index, option);
    if (index.isValid() && hit && index.column() == PackageTableModel::BUTTON_COLUMN) {
      ButtonState s = (ButtonState)(index.data(Qt::UserRole).toInt());
      if (s == Raised) {
        model->setData(index, (int)Pressed, Qt::UserRole);
        emit needsUpdate(index);
      }
    }
  } else if (event->type() == QEvent::MouseButtonRelease) {
    if (index.isValid() && index.column() == PackageTableModel::BUTTON_COLUMN) {
      ButtonState s = (ButtonState)(index.data(Qt::UserRole).toInt());
      if (s == Pressed) {
        model->setData(index, (int)Installing, Qt::UserRole);
        emit needsUpdate(index);
        emit clicked(index);
      }
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void PackageDelegate::initButtonStyleOption(const QModelIndex& index,
                                            const QStyleOptionViewItem& option,
                                            QStyleOptionButton* btnOption) const {
  ButtonState s = (ButtonState)(index.data(Qt::UserRole).toInt());
  if (s == Pressed) {
    btnOption->state |= QStyle::State_Sunken;
  } else {
    btnOption->state |= QStyle::State_Raised;
  }
  btnOption->state |= QStyle::State_Enabled;
  btnOption->rect = option.rect.adjusted(1, 1, -1, -1);
  btnOption->text = m_buttonText;
  QPalette palette = QPalette();
  palette.setBrush(QPalette::ButtonText, Qt::black);
  btnOption->palette = palette;
}

AvailablePackagesViewModel::AvailablePackagesViewModel(QObject* parent)
    : PackagesViewModel(parent) {
}

void AvailablePackagesViewModel::loadPackages() {
  // todo: make packages source configurable
  QNetworkReply* reply = Util::sendGetRequest(
      SilkApp::networkManager(),
      "https://raw.githubusercontent.com/silkedit/packages/master/packages.json");
  connect(reply, &QNetworkReply::finished, this, [=] {
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isNull()) {
      QJsonArray jsonPackages = doc.array();
      QList<Package> packages;
      QSet<Package> installedPkgs = installedPackages();
      for (const QJsonValue& value : jsonPackages) {
        const Package& pkg = Package::fromJson(value);
        if (!installedPkgs.contains(pkg)) {
          packages.append(pkg);
        }
      }
      emit packagesLoaded(packages);
    }
  });
}

QString AvailablePackagesViewModel::buttonText() {
  return tr("Install");
}

QString AvailablePackagesViewModel::TextAfterProcess() {
  return tr("Installed");
}

void AvailablePackagesViewModel::processWithPackage(const QModelIndex& index,
                                                    const core::Package& pkg) {
  // Install the package using npm
  auto npmProcess = new QProcess(this);
  connect(npmProcess, &QProcess::readyReadStandardOutput, this,
          [npmProcess] { qDebug() << npmProcess->readAllStandardOutput(); });
  connect(npmProcess, &QProcess::readyReadStandardError, this,
          [npmProcess] { qWarning() << npmProcess->readAllStandardOutput(); });
  connect(npmProcess, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
          [=](QProcess::ProcessError error) { qWarning("npm error. %d", error); });
  connect(npmProcess,
          static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
          [this, npmProcess, index, pkg](int exitCode, QProcess::ExitStatus exitStatus) {
            npmProcess->deleteLater();

            if (exitStatus == QProcess::CrashExit || exitCode != 0) {
              qWarning("npm install failed");
              emit processFailed(index);
              return;
            }

            QDir node_modules = QDir(QDir::tempPath() + "/node_modules");
            if (!node_modules.exists()) {
              qWarning("temp node_modules directory doesn't exist");
              emit processFailed(index);
              return;
            }

            qDebug("npm install succeeded");
            // move the installed package in tmp to user's packages directory
            if (!QDir(Constants::userPackagesDirPath()).exists()) {
              QDir(Constants::silkHomePath()).mkdir(Constants::packagesDirName());
            }
            bool success =
                node_modules.rename(pkg.name, Constants::userPackagesDirPath() + "/" + pkg.name);
            if (!success) {
              qWarning("Failed to move %s", qPrintable(node_modules.filePath(pkg.name)));
              emit processFailed(index);
              return;
            }

            qDebug("installation succeeded. row: %d", index.row());
            emit processSucceeded(index);
            PluginManager::singleton().loadPackage(pkg.name);
          });
  const QStringList args{"i", "--production", "--prefix", QDir::tempPath(), pkg.githubUrl};
  npmProcess->start(Constants::npmPath(), args);
}

PackagesViewModel::PackagesViewModel(QObject* parent) : QObject(parent) {
}

QSet<core::Package> PackagesViewModel::installedPackages() {
  QStringList directories =
      QDir(Constants::userPackagesDirPath()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  QSet<Package> packages;
  for (const QString& dirName : directories) {
    QFile packageJson(Constants::userPackagesDirPath() + "/" + dirName + "/package.json");
    if (packageJson.exists() && packageJson.open(QIODevice::ReadOnly)) {
      QJsonParseError error;
      auto doc = QJsonDocument::fromJson(packageJson.readAll(), &error);
      if (error.error != QJsonParseError::NoError) {
        qWarning("error when loading package.json. %s", qPrintable(error.errorString()));
        continue;
      }
      packages.insert(Package::fromJson(doc.object()));
    }
  }
  return packages;
}

InstalledPackagesViewModel::InstalledPackagesViewModel(QObject* parent)
    : PackagesViewModel(parent) {
}

void InstalledPackagesViewModel::loadPackages() {
  QList<Package> packages = installedPackages().toList();
  std::sort(packages.begin(), packages.end(),
            [](const Package& p1, const Package& p2) { return p2.name.compare(p1.name); });
  emit packagesLoaded(packages);
}

QString InstalledPackagesViewModel::buttonText() {
  return tr("Remove");
}

QString InstalledPackagesViewModel::TextAfterProcess() {
  return tr("Removed");
}

void InstalledPackagesViewModel::processWithPackage(const QModelIndex& index,
                                                    const core::Package& pkg) {
  QDir pkgDir(Constants::userPackagesDirPath() + "/" + pkg.name);
  if (!pkgDir.exists()) {
    qWarning("%s doesn't exist", qPrintable(pkgDir.absolutePath()));
    emit processFailed(index);
    return;
  }

  bool success = pkgDir.removeRecursively();
  if (success) {
    qDebug("%s removed successfully", qPrintable(pkg.name));
    emit processSucceeded(index);
  } else {
    qWarning("Failed to remove directory: %s", qPrintable(pkgDir.absolutePath()));
    emit processFailed(index);
  }
}
