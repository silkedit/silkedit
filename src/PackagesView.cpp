#include <algorithm>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QAbstractTableModel>
#include <QTableView>
#include <QMovie>
#include <QItemDelegate>
#include <QPainter>
#include <QMouseEvent>

#include "PackagesView.h"
#include "ui_PackagesView.h"

PackagesView::PackagesView(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::PackagesView),
      m_accessManager(new QNetworkAccessManager(this)),
      m_reply(nullptr),
      m_pkgsModel(new PackageTableModel(this)),
      m_delegate(new PackageDelegate(this)) {
  ui->setupUi(this);
  auto indicatorMovie = new QMovie(":/images/indicator.gif");
  ui->indicatorLabel->setMovie(indicatorMovie);
  ui->indicatorLabel->hide();
  ui->tableView->setModel(m_pkgsModel);
  ui->tableView->setItemDelegate(m_delegate);
  connect(m_delegate, &PackageDelegate::needsUpdate,
          [=](const QModelIndex& index) { ui->tableView->update(index); });
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

bool PackageTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role == Qt::UserRole) {
    m_buttonStateMap[index] = (PackageDelegate::ButtonState)value.toInt();
    return true;
  } else {
    return QAbstractTableModel::setData(index, value, role);
  }
}

Package::Package(const QJsonValue& jsonValue) {
  QJsonObject jsonObj = jsonValue.toObject();
  name = jsonObj["name"].toString();
  version = jsonObj["version"].toString();
  description = jsonObj["description"].toString();
  repository = jsonObj["repository"].toString();
}

PackageDelegate::PackageDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

void PackageDelegate::paint(QPainter* painter,
                            const QStyleOptionViewItem& option,
                            const QModelIndex& index) const {
  if (!index.isValid() || index.column() != PackageTableModel::BUTTON_COLUMN) {
    return QStyledItemDelegate::paint(painter, option, index);
  }

  QStyleOptionButton opt;
  initButtonStyleOption(index, option, &opt);
  QApplication::style()->drawControl(QStyle::CE_PushButton, &opt, painter, 0);
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
      model->setData(index, (int)Pressed, Qt::UserRole);
      emit needsUpdate(index);
    }
  } else if (event->type() == QEvent::MouseButtonRelease) {
    if (index.isValid() && index.column() == PackageTableModel::BUTTON_COLUMN) {
      ButtonState s = (ButtonState)(index.data(Qt::UserRole).toInt());
      if (s == Pressed) {
        // emit clicked
        qDebug("clicked");
      }
      model->setData(index, (int)Raised, Qt::UserRole);
      emit needsUpdate(index);
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
