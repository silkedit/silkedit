#include <QtTest/QtTest>

#include "LineSeparator.h"

namespace {}

class LineSeparatorTest : public QObject {
  Q_OBJECT
 private slots:
  void guess() {
    QString textWithNoSeparator = "hoge";
    QCOMPARE(LineSeparator::guess(textWithNoSeparator), LineSeparator::defaultLineSeparator());

    QString winSeparatorText = "hoge\r\n";
    QCOMPARE(LineSeparator::guess(winSeparatorText), LineSeparator::Windows);

    QString unixSeparatorText = "hoge\n";
    QCOMPARE(LineSeparator::guess(unixSeparatorText), LineSeparator::Unix);

    QString classicMacSeparatorText = "hoge\r";
    QCOMPARE(LineSeparator::guess(classicMacSeparatorText), LineSeparator::ClassicMac);

    QString classicMacSeparatorText2 = "hoge\rfoo";
    QCOMPARE(LineSeparator::guess(classicMacSeparatorText2), LineSeparator::ClassicMac);

    QString winAndUnixSeparatorText = "hoge\r\nfoo\n";
    QCOMPARE(LineSeparator::guess(winAndUnixSeparatorText), LineSeparator::Windows);

    QString unixAndWinSeparatorText = "hoge\nfoo\r\n";
    QCOMPARE(LineSeparator::guess(unixAndWinSeparatorText), LineSeparator::Unix);

    QString classicMacAndWinSeparatorText = "hoge\rfoo\r\n";
    QCOMPARE(LineSeparator::guess(classicMacAndWinSeparatorText), LineSeparator::ClassicMac);
  }
};

QTEST_MAIN(LineSeparatorTest)
#include "LineSeparatorTest.moc"
