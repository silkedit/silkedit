#include <QtTest/QtTest>
#include <QStringList>
#include "Util.h"
#include "Url.h"
#include "QObjectUtil.h"

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
};

}  // namespace core

QTEST_MAIN(core::UtilTest)
#include "UtilTest.moc"
