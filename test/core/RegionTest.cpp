#include <QtTest/QtTest>

#include "Region.h"

namespace core {

class RegionTest : public QObject {
  Q_OBJECT
 private slots:
  void intersects();
};

void RegionTest::intersects() {
  // if region is empty, it intersects nothing
  QVERIFY(!Region(0, 0).intersects(Region(0, 0)));
  QVERIFY(!Region(0, 0).intersects(Region(0, 1)));
  QVERIFY(!Region(0, 3).intersects(Region(0, 0)));

  QVERIFY(!Region(3, 5).intersects(Region(1, 2)));
  QVERIFY(!Region(3, 5).intersects(Region(1, 3)));

  // region A is right of region B
  QVERIFY(Region(3, 5).intersects(Region(1, 4)));

  // region A fully covers region B
  QVERIFY(Region(3, 5).intersects(Region(3, 4)));
  QVERIFY(Region(3, 5).intersects(Region(4, 5)));

  // region A is left of region B
  QVERIFY(Region(3, 5).intersects(Region(4, 5)));
  QVERIFY(Region(3, 5).intersects(Region(4, 6)));

  // region B is outside of region A
  QVERIFY(!Region(3, 5).intersects(Region(5, 6)));
  QVERIFY(!Region(3, 5).intersects(Region(6, 7)));

  // region A completely matches region B
  QVERIFY(Region(0, 3).intersects(Region(0, 3)));
}

}  // namespace core

QTEST_MAIN(core::RegionTest)
#include "RegionTest.moc"
