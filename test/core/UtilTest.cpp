#include <QtTest/QtTest>
#include <QStringList>
#include "Util.h"

namespace core {

class UtilTest : public QObject {
  Q_OBJECT
 private slots:
  void binarySearch() {
    QVector<int> vec(0);
    for (int i = 0; i < 100; i++) {
      vec.append(i);
    }
    int idx = Util::binarySearch(vec.length(), [vec](size_t i) { return i > 70; });
    QCOMPARE(idx, 70);

    // binarySearch returns vec.length if there's no element which returns f(i) == true
    idx = Util::binarySearch(vec.length(), [vec](size_t) { return false; });
    QCOMPARE(idx, vec.length());

    // binarySearch returns 0 if all the items satisfies the predicate.
    idx = Util::binarySearch(vec.length(), [vec](size_t i) { return i <= 100; });
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
};

}  // namespace core

QTEST_MAIN(core::UtilTest)
#include "UtilTest.moc"
