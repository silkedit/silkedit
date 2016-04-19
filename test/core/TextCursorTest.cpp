#include <QtTest/QtTest>
#include <QTextDocument>

#include "TextCursor.h"

namespace core {

class TextCursorTest : public QObject {
  Q_OBJECT

 private slots:
  void nextEnglishWord() {
    QTextDocument doc(u8"A brown firefox");
    QTextCursor cursor(&doc);
    int i = 0;
    QList<int> list = {1, 7, 15};
    while (!cursor.atEnd()) {
      TextCursor::customMovePosition(cursor, QTextCursor::NextWord);
      QVERIFY(i < list.size());
      QCOMPARE(cursor.position(), list[i++]);
    }
  }

  void nextJapaneseWord() {
    QTextDocument doc(u8"単語単位に分割する");
    QTextCursor cursor(&doc);
    int i = 0;
    QList<int> list = {2, 4, 5, 7, 9};
    while (!cursor.atEnd()) {
      TextCursor::customMovePosition(cursor, QTextCursor::NextWord);
      QVERIFY(i < list.size());
      QCOMPARE(cursor.position(), list[i++]);
    }
  }

  void previousEnglishWord() {
    QTextDocument doc(u8"A brown firefox");
    QTextCursor cursor(&doc);
    cursor.movePosition(QTextCursor::End);
    int i = 0;
    QList<int> list = {8, 2, 0};
    while (!cursor.atStart()) {
      TextCursor::customMovePosition(cursor, QTextCursor::PreviousWord);
      QVERIFY(i < list.size());
      QCOMPARE(cursor.position(), list[i++]);
    }
  }

  void previousJapaneseWord() {
    QTextDocument doc(u8"単語単位に分割する");
    QTextCursor cursor(&doc);
    cursor.movePosition(QTextCursor::End);
    int i = 0;
    QList<int> list = {7, 5, 4, 2, 0};
    while (!cursor.atStart()) {
      TextCursor::customMovePosition(cursor, QTextCursor::PreviousWord);
      QVERIFY(i < list.size());
      QCOMPARE(cursor.position(), list[i++]);
    }
  }

  // Check original StartOfWord behavior
  void startOfEnglishWord() {
    QTextDocument doc(u8"A brown firefox");
    QTextCursor cursor(&doc);

    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 0);

    cursor.setPosition(2);
    cursor.movePosition(QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 2);

    cursor.setPosition(3);
    cursor.movePosition(QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 2);

    cursor.setPosition(9);
    cursor.movePosition(QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 8);

    cursor.setPosition(15);
    cursor.movePosition(QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 8);
  }

  void startOfJapaneseWord() {
    QTextDocument doc(u8"単語単位に分割する");
    QTextCursor cursor(&doc);

    cursor.setPosition(0);
    TextCursor::customMovePosition(cursor, QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 0);

    cursor.setPosition(2);
    TextCursor::customMovePosition(cursor, QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 2);

    cursor.setPosition(3);
    TextCursor::customMovePosition(cursor, QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 2);

    cursor.setPosition(4);
    TextCursor::customMovePosition(cursor, QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 4);

    cursor.setPosition(9);
    TextCursor::customMovePosition(cursor, QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 7);
  }

  void startOfJapaneseWord2() {
    QTextDocument doc(u8")をベース");
    QTextCursor cursor(&doc);

    cursor.setPosition(1);
    TextCursor::customMovePosition(cursor, QTextCursor::StartOfWord);
    QCOMPARE(cursor.position(), 1);
  }

  // Check original EndOfWord behavior
  void endOfEnglishWord() {
    QTextDocument doc(u8"A brown firefox");
    QTextCursor cursor(&doc);

    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 1);

    cursor.setPosition(2);
    cursor.movePosition(QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 7);

    cursor.setPosition(3);
    cursor.movePosition(QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 7);

    cursor.setPosition(9);
    cursor.movePosition(QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 15);

    cursor.setPosition(15);
    cursor.movePosition(QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 15);
  }

  void endOfJapaneseWord() {
    QTextDocument doc(u8"単語単位に分割する");
    QTextCursor cursor(&doc);

    cursor.setPosition(0);
    TextCursor::customMovePosition(cursor, QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 2);

    cursor.setPosition(2);
    TextCursor::customMovePosition(cursor, QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 2);

    cursor.setPosition(3);
    TextCursor::customMovePosition(cursor, QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 4);

    cursor.setPosition(4);
    TextCursor::customMovePosition(cursor, QTextCursor::EndOfWord);
    QCOMPARE(cursor.position(), 4);
  }

  // Check original WordUnderCursor behavior
  void selectEnglishWord() {
    QTextDocument doc(u8"A brown firefox");
    QTextCursor cursor(&doc);

    cursor.setPosition(0);
    cursor.select(QTextCursor::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QStringLiteral("A"));

    cursor.setPosition(2);
    cursor.select(QTextCursor::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QStringLiteral("brown"));

    cursor.setPosition(9);
    cursor.select(QTextCursor::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QStringLiteral("firefox"));

    cursor.setPosition(15);
    cursor.select(QTextCursor::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QStringLiteral("firefox"));
  }

  void selectJapaneseWord() {
    QTextDocument doc(u8"単語単位に分割する");
    QTextCursor cursor(&doc);

    cursor.setPosition(0);
    TextCursor::customSelect(cursor, QTextCursor::SelectionType::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QString(u8"単語"));

    cursor.setPosition(2);
    TextCursor::customSelect(cursor, QTextCursor::SelectionType::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QString(u8"単位"));

    cursor.setPosition(4);
    TextCursor::customSelect(cursor, QTextCursor::SelectionType::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QString(u8"に"));

    cursor.setPosition(6);
    TextCursor::customSelect(cursor, QTextCursor::SelectionType::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QString(u8"分割"));

    cursor.setPosition(7);
    TextCursor::customSelect(cursor, QTextCursor::SelectionType::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QString(u8"する"));

    cursor.setPosition(9);
    TextCursor::customSelect(cursor, QTextCursor::SelectionType::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QString(u8"する"));
  }

  void selectJapaneseWord2() {
    QTextDocument doc(u8")をベース");
    QTextCursor cursor(&doc);

    cursor.setPosition(1);
    TextCursor::customSelect(cursor, QTextCursor::SelectionType::WordUnderCursor);
    QCOMPARE(cursor.selectedText(), QString(u8"を"));
  }
};

}  // namespace core

QTEST_MAIN(core::TextCursorTest)
#include "TextCursorTest.moc"
