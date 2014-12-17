#include <oniguruma.h>
#include <QtTest/QtTest>

#include "Regexp.h"

class RegexpTest : public QObject {
  Q_OBJECT
 private slots:
  void compile();
  void findStringSubmatchIndex();
};

void RegexpTest::compile()
{
  Regexp* reg = Regexp::compile("(<\\?)\\s*([-_a-zA-Z0-9]+)");
  QVERIFY(reg);
}

void RegexpTest::findStringSubmatchIndex()
{
  Regexp* reg = Regexp::compile("(<\\?)\\s*([-_a-zA-Z0-9]+)");
  QVector<int>* indices = reg->findStringSubmatchIndex(R"(<?xml version="1.0" encoding="UTF-8"?>)");
  QVERIFY(indices);
  QCOMPARE(indices->size(), 6);
  QCOMPARE(*indices, QVector<int>({0, 5, 0, 2, 2, 5}));

  // search fail
  indices = reg->findStringSubmatchIndex("aaa");
  QVERIFY(!indices);
}

QTEST_MAIN(RegexpTest)
#include "RegexpTest.moc"
