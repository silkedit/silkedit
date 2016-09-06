#include <QtTest/QtTest>

#include "Font.h"
#include "QObjectUtil.h"

namespace core {

class QObjectUtilTest : public QObject {
  Q_OBJECT

 public:
  QObjectUtilTest() {
    qRegisterMetaType<QFont>();
    qRegisterMetaType<core::Font*>("Font*");
    qRegisterMetaType<core::Font*>("core::Font*");
  }

 public slots:
  QString fontTestFunc(QFont font) {
    qDebug() << "fontTestFunc called. font" << font;
    return font.family();
  }

 private slots:

  void invokeQObjectMethodInternalWithWrappedTypeArg() {
    auto family = QStringLiteral("Arial");
    QObject* newFont = QObjectUtil::newInstanceFromJS(Font::staticMetaObject,
                                                      QVariantList{QVariant::fromValue(family)});
    try {
      QVariant result = QObjectUtil::invokeQObjectMethodInternal(
          this, "fontTestFunc", QVariantList{QVariant::fromValue(newFont)});
      QVERIFY(result.canConvert<QString>() && result.toString() == family);
    } catch (const std::exception& e) {
      QFAIL(e.what());
    }
  }
};
}  // namespace core

QTEST_MAIN(core::QObjectUtilTest)
#include "QObjectUtilTest.moc"
