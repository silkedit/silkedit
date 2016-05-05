#include <QtTest/QtTest>

#include "Util.h"

namespace {}

class UtilTest : public QObject {
  Q_OBJECT
 private slots:
  void binarySearch();
};

void UtilTest::binarySearch() {
  QVector<int> vec(0);
  for (int i = 0; i < 100; i++) {
    vec.append(i);
  }
  int idx = Util::binarySearch(vec.length(), [vec](int i) { return i > 70; });
  QCOMPARE(idx, 70);

  // binarySearch returns vec.length if there's no element which returns f(i) == true
  idx = Util::binarySearch(vec.length(), [vec](int i) { return i < 0; });
  QCOMPARE(idx, vec.length());
}

QTEST_MAIN(UtilTest)
#include "UtilTest.moc"
