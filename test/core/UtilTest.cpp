#include <QtTest/QtTest>
#include <QStringList>
#include <QCompleter>
#include <QAbstractItemView>

#include "Util.h"
#include "Font.h"
#include "QObjectUtil.h"
#include "JSValue.h"
#include "KeyEvent.h"
#include "QScrollBarWrap.h"

namespace core {

class UtilTest : public QObject {
  Q_OBJECT

 public:
  UtilTest() {
    qRegisterMetaType<QFont>();
    qRegisterMetaType<core::Font*>("Font*");
    qRegisterMetaType<core::Font*>("core::Font*");
    qRegisterMetaType<QKeyEvent*>();
    qRegisterMetaType<core::KeyEvent*>("KeyEvent*");
    qRegisterMetaType<core::KeyEvent*>("core::KeyEvent*");
    qRegisterMetaType<QEvent::Type>("QEvent::Type");
    qRegisterMetaType<core::QScrollBarWrap*>("QScrollBarWrap*");
    qRegisterMetaType<core::QScrollBarWrap*>("core::QScrollBarWrap*");
    qRegisterMetaType<QWidget*>();
  }

 private slots:
  void binarySearch() {
    QVector<int> vec(0);
    for (int i = 0; i < 100; i++) {
      vec.append(i);
    }
    int idx = Util::binarySearch(vec.length(), [vec](int i) { return i > 70; });
    QCOMPARE(idx, 70);

    // binarySearch returns vec.length if there's no element which returns f(i) == true
    idx = Util::binarySearch(vec.length(), [vec](int) { return false; });
    QCOMPARE(idx, vec.length());

    // binarySearch returns 0 if all the items satisfies the predicate.
    idx = Util::binarySearch(vec.length(), [vec](int i) { return i <= 100; });
    QCOMPARE(idx, 0);
  }

  void toStdStringList() {
    const QStringList qStrList = {"user/local", "sys/bin", "var/lib/hoge_fuga"};
    std::list<std::string> stdStringList = Util::toStdStringList(qStrList);
    QCOMPARE((int)stdStringList.size(), qStrList.size());
  }

  void toArgv() {
    // the last element of argv is nullptr
    char** argv = Util::toArgv(QStringList());
    QCOMPARE(argv[0], (char*)(nullptr));

    const QStringList qStrList = {"hoge", "foo", "fuga"};
    argv = Util::toArgv(qStrList);
    QCOMPARE(argv[0], "hoge");
    QCOMPARE(argv[1], "foo");
    QCOMPARE(argv[2], "fuga");
    QCOMPARE(argv[3], (char*)(nullptr));
  }

  void wrappedTypeCheckForWrapper() {
    // Given QObject constructed by QMetaObject::newInstance
    auto family = QStringLiteral("Arial");
    QObject* newFont = QObjectUtil::newInstanceFromJS(Font::staticMetaObject,
                                                      QVariantList{QVariant::fromValue(family)});
    // When
    auto result = Util::wrappedTypeCheck(QVariant::fromValue(newFont), "QFont");

    // Then true
    QVERIFY(result);
  }

  void wrappedTypeCheckForInt() {
    // Given int
    auto result = Util::wrappedTypeCheck(QVariant::fromValue(3), "QFont");

    // Then returns false
    QVERIFY(!result);
  }

  void wrappedTypeCheckForInheritance() {
    // Given KeyEvent
    // KeyEvent class has the following class info
    // Q_CLASSINFO(INHERITS, "QEvent")
    QObject* keyEvent = QObjectUtil::newInstanceFromJS(
        KeyEvent::staticMetaObject,
        QVariantList{QVariant::fromValue(QEvent::KeyPress),
                     QVariant::fromValue(static_cast<int>(Qt::Key_0)),
                     QVariant::fromValue(static_cast<int>(Qt::NoModifier))});
    Q_ASSERT(keyEvent);
    auto result = Util::wrappedTypeCheck(QVariant::fromValue(keyEvent), "QEvent*");

    // Then returns true
    QVERIFY(result);
  }

  void wrappedTypeCheckForQObject() {
    // Given QScrollBarWrap
    QCompleter completer;
    std::unique_ptr<QScrollBarWrap> wrap(
        new QScrollBarWrap(completer.popup()->verticalScrollBar()));
    auto result = Util::wrappedTypeCheck(QVariant::fromValue(wrap.get()), "QWidget*");

    // Then returns true
    QVERIFY(result);
  }

  void convertArgs() {
    auto family = QStringLiteral("Arial");
    QObject* newFont = QObjectUtil::newInstanceFromJS(Font::staticMetaObject,
                                                      QVariantList{QVariant::fromValue(family)});
    QVariantList args = QVariantList{QVariant::fromValue(newFont)};
    bool result = Util::convertArgs(ParameterTypes(), args);
    QVERIFY(result);
    QCOMPARE(args[0].typeName(), "QFont");
  }

  void toVariantNull() {
    auto var = Util::toVariant("null");
    QVERIFY(var.canConvert<JSNull>());

    var = Util::toVariant("NULL");
    QVERIFY(var.canConvert<JSNull>());

    var = Util::toVariant("~");
    QVERIFY(var.canConvert<JSNull>());
  }

  void toVariantBool() {
    auto var = Util::toVariant("true");
    QVERIFY(var.canConvert<bool>());
    QVERIFY(var.toBool());

    var = Util::toVariant("True");
    QVERIFY(var.canConvert<bool>());
    QVERIFY(var.toBool());

    var = Util::toVariant("TRUE");
    QVERIFY(var.canConvert<bool>());
    QVERIFY(var.toBool());

    var = Util::toVariant("false");
    QVERIFY(var.canConvert<bool>());
    QVERIFY(!var.toBool());

    var = Util::toVariant("False");
    QVERIFY(var.canConvert<bool>());
    QVERIFY(!var.toBool());

    var = Util::toVariant("FALSE");
    QVERIFY(var.canConvert<bool>());
    QVERIFY(!var.toBool());
  }

  void toVariantBase10Int() {
    auto var = Util::toVariant("+0");
    QVERIFY(var.canConvert<int>());
    bool ok;
    QCOMPARE(var.toInt(&ok), 0);
    QVERIFY(ok);

    var = Util::toVariant("0");
    QVERIFY(var.canConvert<int>());
    QCOMPARE(var.toInt(&ok), 0);
    QVERIFY(ok);

    var = Util::toVariant("-0");
    QVERIFY(var.canConvert<int>());
    QCOMPARE(var.toInt(&ok), 0);
    QVERIFY(ok);

    var = Util::toVariant("+10");
    QVERIFY(var.canConvert<int>());
    QCOMPARE(var.toInt(&ok), 10);
    QVERIFY(ok);

    var = Util::toVariant("-10000");
    QVERIFY(var.canConvert<int>());
    QCOMPARE(var.toInt(&ok), -10000);
    QVERIFY(ok);
  }

  void toVariantBase8Int() {
    auto var = Util::toVariant("0o0");
    QVERIFY(var.canConvert<int>());
    bool ok;
    QCOMPARE(var.toInt(&ok), 0);
    QVERIFY(ok);

    var = Util::toVariant("0o10");
    QVERIFY(var.canConvert<int>());
    QCOMPARE(var.toInt(&ok), 8);
    QVERIFY(ok);

    var = Util::toVariant("0o123");
    QVERIFY(var.canConvert<int>());
    QCOMPARE(var.toInt(&ok), 83);
    QVERIFY(ok);
  }

  void toVariantBase16Int() {
    auto var = Util::toVariant("0x0a");
    QVERIFY(var.canConvert<int>());
    bool ok;
    QCOMPARE(var.toInt(&ok), 10);
    QVERIFY(ok);

    var = Util::toVariant("0x0aF");
    QVERIFY(var.canConvert<int>());
    QCOMPARE(var.toInt(&ok), 175);
    QVERIFY(ok);
  }

  void toVariantFloat() {
    auto var = Util::toVariant("+.0");
    QVERIFY(var.canConvert<double>());
    bool ok;
    QCOMPARE(var.toDouble(&ok), 0.0);
    QVERIFY(ok);

    var = Util::toVariant("3.04");
    QVERIFY(var.canConvert<double>());
    QCOMPARE(var.toDouble(&ok), 3.04);
    QVERIFY(ok);

    var = Util::toVariant("-3.04");
    QVERIFY(var.canConvert<double>());
    QCOMPARE(var.toDouble(&ok), -3.04);
    QVERIFY(ok);

    var = Util::toVariant("5.28e+1");
    QVERIFY(var.canConvert<double>());
    QCOMPARE(var.toDouble(&ok), 52.8);
    QVERIFY(ok);
  }

  void toVariantInfinity() {
    auto var = Util::toVariant("+.inf");
    QVERIFY(var.canConvert<double>());
    bool ok;
    QVERIFY(std::isinf(var.toDouble(&ok)));
    QVERIFY(ok);

    var = Util::toVariant("-.INF");
    QVERIFY(var.canConvert<double>());
    QVERIFY(std::isinf(var.toDouble(&ok)));
    QVERIFY(ok);
  }

  void toVariantNan() {
    auto var = Util::toVariant(".nan");
    QVERIFY(var.canConvert<double>());
    bool ok;
    QVERIFY(std::isnan(var.toDouble(&ok)));
    QVERIFY(ok);

    var = Util::toVariant(".NAN");
    QVERIFY(var.canConvert<double>());
    QVERIFY(std::isnan(var.toDouble(&ok)));
    QVERIFY(ok);
  }
};

}  // namespace core

QTEST_MAIN(core::UtilTest)
#include "UtilTest.moc"
