#include <algorithm>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QAbstractTableModel>
#include <QTableView>
#include <QMovie>

#include "PackagesView.h"
#include "ui_PackagesView.h"

PackagesView::PackagesView(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::PackagesView),
      m_accessManager(new QNetworkAccessManager(this)),
      m_reply(nullptr),
      m_pkgsModel(new PackageTableModel(this)) {
  ui->setupUi(this);
  auto indicatorMovie = new QMovie(":/images/indicator.gif");
  ui->indicatorLabel->setMovie(indicatorMovie);
  ui->indicatorLabel->hide();
  ui->tableView->setModel(m_pkgsModel);
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
  m_reply = m_accessManager->get(QNetworkRequest(
      QUrl("https://raw.githubusercontent.com/silkedit/packages/master/packages.json")));
  connect(m_reply,
          static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
          this, [=](QNetworkReply::NetworkError) { handleError(m_reply); });
  connect(m_reply, &QNetworkReply::sslErrors, this, [=](QList<QSslError> errors) {
    for (QSslError e : errors) {
      qWarning("SSL error: %s", qPrintable(e.errorString()));
    }
  });
  connect(m_reply, &QNetworkReply::finished, this, [=] {
    Q_ASSERT(m_reply->isFinished());
    stopAnimation();

    if (m_reply->error() != QNetworkReply::NoError) {
      handleError(m_reply);
      return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(m_reply->readAll());
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
  return 3;
}

QVariant PackageTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || role != Qt::DisplayRole || index.row() >= m_packages.size() ||
      index.column() >= Package::ITEM_COUNT) {
    return QVariant();
  }

  const Package& pkg = m_packages[index.row()];
  switch (index.column()) {
    case 0:
      return pkg.name;
    case 1:
      return pkg.version;
    case 2:
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

Package::Package(const QJsonValue& jsonValue) {
  QJsonObject jsonObj = jsonValue.toObject();
  name = jsonObj["name"].toString();
  version = jsonObj["version"].toString();
  description = jsonObj["description"].toString();
  repository = jsonObj["repository"].toString();
}
