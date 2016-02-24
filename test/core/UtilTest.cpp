#include <QtTest/QtTest>
#include <QStringList>
#include "Util.h"
#include "Url.h"
#include "QObjectUtil.h"
#include "JSValue.h"

namespace core {

class UtilTest : public QObject {
  Q_OBJECT

 public:
  UtilTest() {
    qRegisterMetaType<QUrl>();
    qRegisterMetaType<core::Url*>("Url*");
    qRegisterMetaType<core::Url*>("core::Url*");
  }

 private slots:
  void binarySearch() {
    QVector<int> vec(0);
    for (int i = 0; i < 100; i++) {
      vec.append(i);
    }
    size_t idx = Util::binarySearch(vec.length(), [vec](size_t i) { return i > 70; });
    QCOMPARE(static_cast<int>(idx), 70);

    // binarySearch returns vec.length if there's no element which returns f(i) == true
    idx = Util::binarySearch(vec.length(), [vec](size_t) { return false; });
    QCOMPARE(static_cast<int>(idx), vec.length());

    // binarySearch returns 0 if all the items satisfies the predicate.
    idx = Util::binarySearch(vec.length(), [vec](size_t i) { return i <= 100; });
    QCOMPARE(static_cast<int>(idx), 0);
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
    auto url = QStringLiteral("url");
    QObject* newUrl = QObjectUtil::newInstanceFromJS(Url::staticMetaObject,
                                                     QVariantList{QVariant::fromValue(url)});
    // When
    auto result = Util::wrappedTypeCheck(QVariant::fromValue(newUrl), "QUrl");

    // Then true
    QVERIFY(result);
  }

  void wrappedTypeCheckForInt() {
    // Given int
    auto result = Util::wrappedTypeCheck(QVariant::fromValue(3), "QUrl");

    // Then returns false
    QVERIFY(!result);
  }

  void convertArgs() {
    auto url = QStringLiteral("url");
    QObject* newUrl = QObjectUtil::newInstanceFromJS(Url::staticMetaObject,
                                                     QVariantList{QVariant::fromValue(url)});
    QVariantList args = QVariantList{QVariant::fromValue(newUrl)};
    bool result = Util::convertArgs(ParameterTypes(), args);
    QVERIFY(result);
    QCOMPARE(args[0].typeName(), "QUrl");
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
