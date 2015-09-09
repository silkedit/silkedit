#include <QObject>
#include <QString>
#include <QtTest/QtTest>

#include "TextEditViewLogic.h"
#include "Regexp.h"
#include "Metadata.h"

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

  void increaseIndentPattern() {
    // Given
    // "if (true) {" matches increaseIndentPattern (doesn't match bracketIndentNextLinePattern)
    const QString text = "if (true) {\n";
    QTextDocument doc;
    doc.setPlainText(text);
    QTextCursor cursor(&doc);
    // cursor is at the end of the document
    cursor.movePosition(QTextCursor::End);
    int tabWidth = 2;
    QString prevLineText = "if (true) {";
    boost::optional<QString> prevPrevLineText = boost::none;
    Metadata::load("testdata/CppIndentation Rules.tmPreferences");
    Metadata* metadata = Metadata::get("source.c++");

    // When
    TextEditViewLogic::indentCurrentLine(&doc, cursor, prevLineText, prevPrevLineText, metadata,
                                         false, tabWidth);

    // Then
    // Current line should be indented.
    const QString expectedText =
        "if (true) {\n"
        "\t";
    QCOMPARE(doc.toPlainText(), expectedText);
  }

  void bracketIndentNextLinePattern() {
    // Given
    // "if (true)" matches bracketIndentNextLinePattern (doesn't match increaseIndentPattern)
    const QString text =
        "if (true)\n"
        "\thoge\n";
    QTextDocument doc;
    doc.setPlainText(text);
    QTextCursor cursor(&doc);
    // cursor is at the end of the document
    cursor.movePosition(QTextCursor::End);
    int tabWidth = 2;
    QString prevLineText = "\thoge";
    QString prevPrevLineText = "if (true)";
    Metadata::load("testdata/CppIndentation Rules.tmPreferences");
    Metadata* metadata = Metadata::get("source.c++");

    // When
    TextEditViewLogic::indentCurrentLine(&doc, cursor, prevLineText, prevPrevLineText, metadata,
                                         false, tabWidth);

    // Then
    // Current line should NOT be indented.
    // bracketIndentNextLinePattern makes ONLY the next line indent
    const QString expectedText =
        "if (true)\n"
        "\thoge\n";
    QCOMPARE(doc.toPlainText(), expectedText);
  }

  void BothIncreaseIndentPatternAndBracketIndentNextLinePatternMatched() {
    // Given
    // "if (true) {" matches both increaseIndentPattern and bracketIndentNextLinePattern
    const QString text =
        "if (true) {\n"
        "\thoge\n";
    QTextDocument doc;
    doc.setPlainText(text);
    QTextCursor cursor(&doc);
    // cursor is at the end of the document
    cursor.movePosition(QTextCursor::End);
    int tabWidth = 2;
    QString prevLineText = "\thoge";
    QString prevPrevLineText = "if (true) {";
    Metadata::load("testdata/CppIndentation Rules.tmPreferences");
    Metadata* metadata = Metadata::get("source.c++");

    // When
    TextEditViewLogic::indentCurrentLine(&doc, cursor, prevLineText, prevPrevLineText, metadata,
                                         false, tabWidth);

    // Then
    // Current line should be indented.
    // bracketIndentNextLinePattern should be ignored when increaseIndentPattern is effective.
    const QString expectedText =
        "if (true) {\n"
        "\thoge\n"
        "\t";
    QCOMPARE(doc.toPlainText(), expectedText);
  }

  void alignTheCurrentLineBasedOnPrevLine() {
    // Given
    const QString text = "\t\n";
    QTextDocument doc;
    doc.setPlainText(text);
    QTextCursor cursor(&doc);
    // cursor is at the end of the document
    cursor.movePosition(QTextCursor::End);
    int tabWidth = 2;
    QString prevLineText = "\t";
    boost::optional<QString> prevPrevLineText = boost::none;
    Metadata* metadata = nullptr;

    // When
    TextEditViewLogic::indentCurrentLine(&doc, cursor, prevLineText, prevPrevLineText, metadata,
                                         false, tabWidth);

    // Then
    // Current line should be aligned with the previous line.
    const QString expectedText =
        "\t\n"
        "\t";
    QCOMPARE(doc.toPlainText(), expectedText);
  }
};

QTEST_MAIN(TextEditViewLogicTest)
#include "TextEditViewLogicTest.moc"
