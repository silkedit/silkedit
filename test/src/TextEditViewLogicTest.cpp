#include <QObject>
#include <QString>
#include <QtTest/QtTest>

#include "TextEditViewLogic.h"
#include "Regexp.h"

class TextEditViewLogicTest : public QObject {
  Q_OBJECT
 private slots:
  void isOutdentNecessary() {
    //    QString text = R"(
    //  {
    //    hoge
    //    )";
    QString currentLineText = "    }";
    QString prevLineText = "    hoge";
    // NOTE: If we use raw string literal like R"(^.*\{[^}"']*$)", a link error occurs.
    Regexp* increaseIndentPattern = Regexp::compile("^.*\\{[^}\"']*$");
    auto decreaseIndentPattern = Regexp::compile("^(.*\\*/)?\\s*\\}.*$");
    bool atBlockEnd = true;
    int tabWidth = 2;

    // When
    bool result =
        TextEditViewLogic::isOutdentNecessary(increaseIndentPattern, decreaseIndentPattern,
                                              currentLineText, prevLineText, atBlockEnd, tabWidth);
    // Then
    QVERIFY(result);
  }

  void outdentWithWhitespace() {
    const QString text = R"(
  {
    hoge
    })";
    QTextDocument doc(text);
    QTextCursor cursor(&doc);
    cursor.movePosition(QTextCursor::End);
    int tabWidth = 2;

    // When
    TextEditViewLogic::outdent(&doc, cursor, tabWidth);

    // Then
    const QString expectedText = R"(
  {
    hoge
  })";
    QCOMPARE(doc.toPlainText(), expectedText);
  }

  void outdentWithTab() {
    // We use \t and \n here because Qt Creator converts a tab with space automatically.
    const QString text =
        "\t{\n"
        "\t\thoge\n"
        "\t\t}";
    QTextDocument doc;
    doc.setPlainText(text);
    QTextCursor cursor(&doc);
    cursor.movePosition(QTextCursor::End);
    int tabWidth = 2;

    // When
    TextEditViewLogic::outdent(&doc, cursor, tabWidth);

    // Then
    const QString expectedText =
        "\t{\n"
        "\t\thoge\n"
        "\t}";
    QCOMPARE(doc.toPlainText(), expectedText);
  }
};

QTEST_MAIN(TextEditViewLogicTest)
#include "TextEditViewLogicTest.moc"
