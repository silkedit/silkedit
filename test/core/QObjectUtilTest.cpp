#include <QtTest/QtTest>

#include "Url.h"
#include "QObjectUtil.h"

namespace core {

class QObjectUtilTest : public QObject {
  Q_OBJECT

 public:
  QObjectUtilTest() {
    qRegisterMetaType<QUrl>();
    qRegisterMetaType<core::Url*>("Url*");
    qRegisterMetaType<core::Url*>("core::Url*");
  }

 public slots:
  QString urlTestFunc(QUrl url) {
    qDebug() << "urlTestFunc called. url" << url;
    return url.toString();
  }

 private slots:

  void invokeQObjectMethodInternalWithWrappedTypeArg() {
      auto url = QStringLiteral("url");
      QObject* newUrl = QObjectUtil::newInstanceFromJS(Url::staticMetaObject,
                                                           QVariantList{QVariant::fromValue(url)});
      try {
        QVariant result = QObjectUtil::invokeQObjectMethodInternal(
            this, "urlTestFunc", QVariantList{QVariant::fromValue(newUrl)});
        QVERIFY(result.canConvert<QString>() && result.toString() == url);
      } catch (const std::exception& e) {
        QFAIL(e.what());
      }
  }
};
}  // namespace core

QTEST_MAIN(core::QObjectUtilTest)
#include "QObjectUtilTest.moc"
