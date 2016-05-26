#include <QtTest/QtTest>

#include "TestUtil.h"

void TestUtil::compareLineByLine(const QString &str1, const QString &str2) {
  QStringList list1 = str1.trimmed().split('\n');
  QStringList list2 = str2.trimmed().split('\n');
  if (str1.trimmed().size() != str2.trimmed().size()) {
    qDebug().noquote() << str1;
  }
  QCOMPARE(list1.size(), list2.size());

  for (int i = 0; i < list1.size(); i++) {
    QCOMPARE(list1.at(i).trimmed(), list2.at(i).trimmed());
  }
}
