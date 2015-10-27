#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
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

#include "PackagesView.h"
#include "ui_PackagesView.h"
#include "core/Constants.h"
#include "core/scoped_guard.h"

using core::Package;
using core::Constants;
using core::scoped_guard;

PackagesView::PackagesView(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::PackagesView),
      m_accessManager(new QNetworkAccessManager(this)),
      m_reply(nullptr),
      m_pkgsModel(new PackageTableModel(this)),
      m_delegate(new PackageDelegate(this)) {
  ui->setupUi(this);
  QMovie* indicatorMovie = new QMovie(":/images/indicator.gif", QByteArray(), this);
  ui->indicatorLabel->setMovie(indicatorMovie);
  ui->indicatorLabel->hide();
  ui->tableView->setModel(m_pkgsModel);
  ui->tableView->setItemDelegate(m_delegate);
  connect(m_delegate, &PackageDelegate::needsUpdate,
          [=](const QModelIndex& index) { ui->tableView->update(index); });
  connect(m_delegate, &PackageDelegate::clicked, this, &PackagesView::startDownloadingPackage);
  connect(this, &PackagesView::downloadFinished, this, &PackagesView::installPackage);
  connect(this, &PackagesView::installationFailed, [&](const QModelIndex& index) {
    qDebug("installation failed. row: %d", index.row());
    m_delegate->stopMovie(index.row());
    m_pkgsModel->setData(index, (int)PackageDelegate::Raised, Qt::UserRole);
    ui->tableView->update(index);
  });
  connect(this, &PackagesView::installationSucceeded, [&](const QModelIndex& index) {
    qDebug("installation succeeded. row: %d", index.row());
    m_delegate->stopMovie(index.row());
    m_pkgsModel->setData(index, (int)PackageDelegate::Installed, Qt::UserRole);
    ui->tableView->update(index);
  });
  setLayout(ui->rootHLayout);
}

PackagesView::~PackagesView() {
  qDebug("~PackagesView");
  delete ui;
}

void PackagesView::startLoading() {
  ui->tableView->hide();
  startAnimation();

  //   Note: QNetworkReply objects that are returned from QNetworkAccessManager have this object set
  //   as their parents
  // todo: make packages source configurable
  QNetworkReply* reply =
      sendGetRequest("https://raw.githubusercontent.com/silkedit/packages/master/packages.json");
  connect(reply, &QNetworkReply::finished, this, [=] {
    reply->deleteLater();
    stopAnimation();

    if (reply->error() != QNetworkReply::NoError) {
      handleError(reply);
      return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isNull()) {
      QJsonArray jsonPackages = doc.array();
      QList<Package> packages;
      std::transform(jsonPackages.constBegin(), jsonPackages.constEnd(),
                     std::back_inserter(packages), &Package::fromJson);
      ui->tableView->show();
      m_pkgsModel->setPackages(packages);
    }
  });

  // todo: set timeout
}

void PackagesView::showEvent(QShowEvent*) {
  startLoading();
}

void PackagesView::hideEvent(QHideEvent*) {
  // todo: Cancel requests
}

void PackagesView::handleError(QNetworkReply* reply) {
  Q_ASSERT(reply);
  qWarning("network error: %s", qPrintable(reply->errorString()));
}

void PackagesView::startAnimation() {
  ui->indicatorLabel->show();
  ui->indicatorLabel->movie()->start();
}

void PackagesView::stopAnimation() {
  ui->indicatorLabel->hide();
  ui->indicatorLabel->movie()->stop();
}

QNetworkReply* PackagesView::sendGetRequest(const QString& url) {
  return sendGetRequest(QUrl(url));
}

QNetworkReply* PackagesView::sendGetRequest(const QUrl& url) {
  QNetworkReply* reply = m_accessManager->get(QNetworkRequest(url));
  connect(reply,
          static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
          this, [=](QNetworkReply::NetworkError) { handleError(reply); });
  connect(reply, &QNetworkReply::sslErrors, this, [=](QList<QSslError> errors) {
    for (QSslError e : errors) {
      qWarning("SSL error: %s", qPrintable(e.errorString()));
    }
  });
  return reply;
}

void PackagesView::startDownloadingPackage(const QModelIndex& index) {
  auto pkgOpt = m_pkgsModel->package(index.row());
  if (!pkgOpt) {
    qWarning("package not found. row: %d", index.row());
    emit installationFailed(index);
    return;
  }

  std::unique_ptr<QMovie> indicatorMovie(new QMovie(":/images/indicator.gif", QByteArray(), this));
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
    emit installationFailed(index);
    return;
  }
  QString zipUrlStr = pkg.zipUrl();
  if (zipUrlStr.isEmpty()) {
    qWarning("zip url is empty");
    emit installationFailed(index);
    return;
  }

  // Start downloading a package content as zip
  qDebug("Github zip url: %s", qPrintable(zipUrlStr));
  QNetworkReply* reply = sendGetRequest(zipUrlStr);
  connect(reply, &QNetworkReply::finished, this,
          [this, reply, index, pkg] { downloadPackage(reply, index, pkg); });
}

void PackagesView::downloadPackage(QNetworkReply* reply,
                                   const QModelIndex& index,
                                   const Package& pkg) {
  qWarning("Finished getting redirect url");
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    emit installationFailed(index);
    return;
  }

  // Handle zip url redirection.
  // https://developer.github.com/v3/repos/contents/#get-archive-link
  QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
  if (redirectUrl.isEmpty()) {
    qWarning("redirectUrl is empty");
    emit installationFailed(index);
    return;
  }
  qDebug("redirect url: %s", qPrintable(redirectUrl.toString()));

  QNetworkReply* zipReply = sendGetRequest(redirectUrl);
  QFile* tmpZipFile = new QFile(QDir::temp().filePath(pkg.name + ".zip"), zipReply);
  if (!tmpZipFile->open(QIODevice::ReadWrite)) {
    qWarning("failed to create a tmp zip file. pkg: %s", qPrintable(pkg.name));
    emit installationFailed(index);
    return;
  }

  // Save content to temp zip file
  connect(zipReply, &QNetworkReply::readyRead, this, [zipReply, tmpZipFile] {
    qint64 availableSize = zipReply->bytesAvailable();
    const QByteArray& bytesRead = zipReply->read(availableSize);
    qint64 size = tmpZipFile->write(bytesRead);
    if (size != availableSize) {
      qWarning("writing to a zip file failed. size: %lld, availableSize: %lld", size,
               availableSize);
    }
  });

  connect(zipReply, &QNetworkReply::finished, this, [this, tmpZipFile, zipReply, index, pkg] {
    qDebug("finished downloading content as zip");
    tmpZipFile->flush();
    emit downloadFinished(tmpZipFile, zipReply, index, pkg.name);
  });
}

void PackagesView::installPackage(QFile* tmpZipFile,
                                  QNetworkReply* zipReply,
                                  const QModelIndex& index,
                                  const QString& pkgName) {
  zipReply->deleteLater();
  if (zipReply->error() != QNetworkReply::NoError) {
    emit installationFailed(index);
    return;
  }

  QuaZip zip(tmpZipFile);

  scoped_guard guard([&zip, tmpZipFile] {
    zip.close();
    tmpZipFile->remove();
  });

  if (!zip.open(QuaZip::mdUnzip)) {
    qWarning("failed to unzip content");
    emit installationFailed(index);
    return;
  }

  QDir userPackagesDir(Constants::userPackagesDirPath());
  zip.goToFirstFile();
  const QString& rootDirName = zip.getCurrentFileName();
  if (!rootDirName.endsWith("/")) {
    qWarning("Can't find root dir. rootDirName: %s", qPrintable(rootDirName));
    emit installationFailed(index);
    return;
  }

  // Create root directory
  userPackagesDir.mkpath(rootDirName);
  zip.goToNextFile();

  // Copy contents in a zip file to user's package directory
  bool success = copyContentsInZip(zip, userPackagesDir);
  if (!success) {
    emit installationFailed(index);
    return;
  }

  // Change root directory name to package's name because root dir name in a zip file is wrong.
  userPackagesDir.rename(rootDirName, pkgName);

  emit installationSucceeded(index);
}

bool PackagesView::copyContentsInZip(QuaZip& zip, const QDir& userPackagesDir) {
  QuaZipFile file(&zip);
  for (bool more = true; more; more = zip.goToNextFile()) {
    const QString& entry = zip.getCurrentFileName();
    if (entry.endsWith("/")) {
      // create a directory
      userPackagesDir.mkpath(entry);
    } else {
      // copy a file
      file.open(QIODevice::ReadOnly);
      scoped_guard fileGuard([&file] { file.close(); });
      QFile newFile(userPackagesDir.filePath(entry));
      if (!newFile.open(QIODevice::ReadWrite)) {
        qWarning("Failed to open %s", qPrintable(newFile.fileName()));
        return false;
      }

      qint64 availableSize = file.bytesAvailable();
      qint64 size = newFile.write(file.readAll());
      if (size != availableSize) {
        qWarning("writing to a zip file failed. size: %lld, availableSize: %lld", size,
                 availableSize);
        return false;
      }
      newFile.flush();
    }
  }

  return true;
}

PackageTableModel::PackageTableModel(QObject* parent) : QAbstractTableModel(parent) {
}

void PackageTableModel::setPackages(const QList<Package>& packages) {
  beginResetModel();
  m_packages = packages;
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

PackageDelegate::PackageDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

void PackageDelegate::setMovie(int row, std::unique_ptr<QMovie> movie) {
  m_rowMovieMap[row] = std::move(movie);
}

void PackageDelegate::stopMovie(int row) {
  m_rowMovieMap.erase(row);
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
                                          "Installed", QPalette::WindowText);
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
  QString text("Install");
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
  btnOption->text = tr("Install");
  QPalette palette = QPalette();
  palette.setBrush(QPalette::ButtonText, Qt::black);
  btnOption->palette = palette;
}
