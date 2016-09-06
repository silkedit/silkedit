#include <QtTest/QtTest>
#include <QTextDocument>

#include "LanguageParser.h"
#include "SyntaxHighlighter.h"
#include "Theme.h"
#include "core/Config.h"
#include "TestUtil.h"

namespace core {

namespace {
void checkRegion(const Node& node, Region region) {
  if (!region.fullyCovers(node.region)) {
    qWarning("%s doesn't fully cover %s", qPrintable(region.toString()),
             qPrintable(node.region.toString()));
    QFAIL("");
  }

  for (auto child : node.children) {
    checkRegion(child, region);
  }
}
}

class SyntaxHighlighterTest : public QObject {
  Q_OBJECT
 private:
  Theme* theme = Theme::loadTheme("testdata/Solarized (Dark).tmTheme");
  QFont font = QFont("Helvetica", 12);

 private slots:

  void initTestCase() {
    qRegisterMetaType<QList<core::Node>>("QList<Node>");
    qRegisterMetaType<QList<core::Node>>("QList<core::Node>");
    qRegisterMetaType<core::RootNode>("RootNode");
    qRegisterMetaType<core::RootNode>("core::RootNode");
    qRegisterMetaType<core::LanguageParser>("LanguageParser");
    qRegisterMetaType<core::LanguageParser>("core::LanguageParser");
    qRegisterMetaType<core::Region>("Region");
    qRegisterMetaType<core::Region>("core::Region");
    qRegisterMetaType<core::SyntaxHighlighter*>("SyntaxHighlighter*");
    qRegisterMetaType<core::SyntaxHighlighter*>("core::SyntaxHighlighter*");
  }

  void scopeExtent() {
    const QVector<QString> files(
        {"testdata/grammers/Property List (XML).tmLanguage", "testdata/grammers/XML.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/plist.tmlang");
    QVERIFY(file.open(QIODevice::ReadOnly));

    QTextStream in(&file);
    QTextDocument doc(in.readAll());
    std::unique_ptr<LanguageParser> plistParser(
        LanguageParser::create("text.xml.plist", doc.toPlainText()));
    SyntaxHighlighter plistHighlighter(&doc, std::move(plistParser), theme, font);
    QSignalSpy spy(&plistHighlighter, &SyntaxHighlighter::parseFinished);

    QVERIFY(spy.wait());

    // When
    Region region = plistHighlighter.scopeExtent(10);
    QCOMPARE(region.begin(), 5);
    QCOMPARE(region.end(), 13);
    QCOMPARE(plistHighlighter.scopeName(10),
             QString("text.xml.plist meta.tag.preprocessor.xml entity.other.attribute-name.xml"));

    region = plistHighlighter.scopeExtent(14);
    QCOMPARE(region.begin(), 14);
    QCOMPARE(region.end(), 15);
    QCOMPARE(plistHighlighter.scopeName(14),
             QString("text.xml.plist meta.tag.preprocessor.xml string.quoted.double.xml "
                     "punctuation.definition.string.begin.xml"));

    // XML

    std::unique_ptr<LanguageParser> xmlParser(
        LanguageParser::create("text.xml", doc.toPlainText()));
    SyntaxHighlighter xmlHighlighter(&doc, std::move(xmlParser), theme, font);
    QSignalSpy xmlSpy(&xmlHighlighter, &SyntaxHighlighter::parseFinished);

    QVERIFY(xmlSpy.wait());

    // Node at 148 has an empty name like this

    // 142-163: "meta.tag.xml"
    //   142-143: "punctuation.definition.tag.xml" - Data: "<"
    //   143-148: "entity.name.tag.localname.xml" - Data: "plist"
    //   148-157: ""
    //     149-156: "entity.other.attribute-name.localname.xml" - Data: "version"
    region = xmlHighlighter.scopeExtent(148);
    QCOMPARE(region.begin(), 148);
    QCOMPARE(region.end(), 157);
    QCOMPARE(xmlHighlighter.scopeName(148), QString("text.xml meta.tag.xml"));

    region = xmlHighlighter.scopeExtent(149);
    QCOMPARE(region.begin(), 149);
    QCOMPARE(region.end(), 156);
    QCOMPARE(xmlHighlighter.scopeName(149),
             QString("text.xml meta.tag.xml entity.other.attribute-name.localname.xml"));
  }

  void updateNode() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QString text = QString(R"(
class hoge {
  void foo();
};
)").trimmed();
    QTextDocument* doc = new QTextDocument(text);
    std::unique_ptr<LanguageParser> parser(
        LanguageParser::create("source.c++", doc->toPlainText()));
    SyntaxHighlighter cppHighlighter(doc, std::move(parser), theme, font);
    QSignalSpy spy(&cppHighlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    checkRegion(cppHighlighter.rootNode(), cppHighlighter.rootNode().region);

    QTextCursor cursor(cppHighlighter.document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n");
    // needs to call updateNode manually because of this bug
    // https://bugreports.qt.io/browse/QTBUG-43695
    cppHighlighter.updateNode(text.length(), 0, 1);
    QVERIFY(spy.wait());
    checkRegion(cppHighlighter.rootNode(), cppHighlighter.rootNode().region);

    cursor.movePosition(QTextCursor::End);
    QString str = "class aa {";
    cursor.insertText(str);
    cppHighlighter.updateNode(text.length() + 1, 0, str.length());
    QVERIFY(spy.wait());
    checkRegion(cppHighlighter.rootNode(), cppHighlighter.rootNode().region);

    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n");
    cppHighlighter.updateNode(text.length() + 1 + str.length(), 0, 1);
    QVERIFY(spy.wait());
    checkRegion(cppHighlighter.rootNode(), cppHighlighter.rootNode().region);
  }

  void updateNodeWithPaste() {
    const QVector<QString> files({"testdata/grammers/CSS.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QString text = QString(R"(
StatusBar QComboBox::down-arrow {
    /*image: url(noimg);*/
    border-width: 0px;
}
)").trimmed();
    QTextDocument doc(text);
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.css", doc.toPlainText()));
    SyntaxHighlighter highlighter(&doc, std::move(parser), theme, font);
    QSignalSpy spy(&highlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());
    //    qDebug().noquote() << highlighter.rootNode()->toString();
    //    qDebug().noquote() << highlighter.asHtml();

    QTextCursor cursor(&doc);
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    highlighter.updateNode(0, text.length(), 0);
    QVERIFY(spy.wait());
    QVERIFY(doc.isEmpty());
    cursor.insertText(text);
    // https://bugreports.qt.io/browse/QTBUG-3495
    highlighter.updateNode(0, 1, text.length() + 1);
    QVERIFY(spy.wait());
    QCOMPARE(doc.toPlainText(), text);

    QFile output("testdata/highlighter_test/updateNodeWithPasteResult.html");
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream resIn(&output);
    QCOMPARE(highlighter.asHtml(), resIn.readAll());
  }

  void cssHighlightTest() {
    const QVector<QString> files({"testdata/grammers/CSS.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QString text = QString(R"(
StatusBar QComboBox::down-arrow {
    /*image: url(noimg);*/
    border-width: 0px;
}
)").trimmed();
    QTextDocument doc(text);
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.css", doc.toPlainText()));
    SyntaxHighlighter highlighter(&doc, std::move(parser), theme, font);
    QSignalSpy spy(&highlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    QTextCursor cursor(&doc);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
    cursor.deletePreviousChar();
    highlighter.updateNode(32, 1, 0);
    QVERIFY(spy.wait());

    QFile resFile("testdata/highlighter_test/cssHighlightTestAfterDeletion.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()),
                                resIn.readAll());

    cursor.insertText("{");
    highlighter.updateNode(32, 0, 1);
    QVERIFY(spy.wait());
    QCOMPARE(doc.toPlainText(), text);

    QString result = QString(R"r(
0-85: "source.css"
  0-32: "meta.selector.css" - Data: "StatusBar QComboBox::down-arrow "
  32-85: ""
    32-85: "meta.property-list.css"
      32-33: "punctuation.section.property-list.begin.css" - Data: "{"
      38-60: "comment.block.css"
        38-40: "punctuation.definition.comment.css" - Data: "/*"
        58-60: "punctuation.definition.comment.css" - Data: "*/"
      65-77: "meta.property-name.css"
        65-77: "support.type.property-name.css" - Data: "border-width"
      77-83: "meta.property-value.css"
        77-78: "punctuation.separator.key-value.css" - Data: ":"
        79-82: "constant.numeric.css"
          80-82: "keyword.other.unit.css" - Data: "px"
        82-83: "punctuation.terminator.rule.css" - Data: ";"
      84-85: "punctuation.section.property-list.end.css" - Data: "}"
)r").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()), result);

    //    qDebug().noquote() << highlighter.rootNode()->toString();
    //    qDebug().noquote() << highlighter.asHtml();

    QFile output("testdata/highlighter_test/updateNodeWithPasteResult.html");
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream resInOutput(&output);
    QCOMPARE(highlighter.asHtml(), resInOutput.readAll());
  }

  void changeThemeTest() {
    const QVector<QString> files({"testdata/grammers/CSS.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QString text = QString(R"(
StatusBar QComboBox::down-arrow {
    /*image: url(noimg);*/
    border-width: 0px;
}
)").trimmed();
    QTextDocument doc(text);
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.css", doc.toPlainText()));
    SyntaxHighlighter highlighter(&doc, std::move(parser), theme, font);
    QSignalSpy spy(&highlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    Theme* monokai = Theme::loadTheme("testdata/Monokai.tmTheme");
    QVERIFY(monokai);
    Config::singleton().setTheme(monokai, true);
    //    qDebug().noquote() << highlighter.asHtml();

    QFile output("testdata/highlighter_test/changeThemeTestResult.html");
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream resInOutput(&output);
    QCOMPARE(highlighter.asHtml(), resInOutput.readAll());
  }

  void cppHighlightTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QFile file("testdata/highlighter_test/cppHighlightTestInput.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    const auto& text = in.readAll();
    QTextDocument doc(text);
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.c++", doc.toPlainText()));
    SyntaxHighlighter highlighter(&doc, std::move(parser), theme, font);
    QSignalSpy spy(&highlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    QFile output("testdata/highlighter_test/cppHighlightTestInput.res");
    QVERIFY(output.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream resInOutput(&output);
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()),
                                resInOutput.readAll());

    QTextCursor cursor(&doc);
    cursor.deleteChar();
    highlighter.updateNode(0, 1, 0);
    QVERIFY(spy.wait());

    QString result = QString(R"r(
0-30: "source.c++"
  0-15: "meta.preprocessor.c.include"
    2-9: "keyword.control.import.include.c" - Data: "include"
    10-15: "string.quoted.double.include.c"
      10-11: "punctuation.definition.string.begin.c" - Data: """
      14-15: "punctuation.definition.string.end.c" - Data: """
  16-30: "meta.preprocessor.c.include"
    17-24: "keyword.control.import.include.c" - Data: "include"
    25-30: "string.quoted.double.include.c"
      25-26: "punctuation.definition.string.begin.c" - Data: """
      29-30: "punctuation.definition.string.end.c" - Data: """

)r").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()), result);
  }

  void pasteTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QString text = QString(R"(
#include <QWidget>
#include <QApplication>
#include "core/macros.h"
)").trimmed();
    QTextDocument doc(text);
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.c++", doc.toPlainText()));
    SyntaxHighlighter highlighter(&doc, std::move(parser), theme, font);
    QSignalSpy spy(&highlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    QFile output("testdata/highlighter_test/cppHighlightTestInput.res");
    QString result = QString(R"(
0-67: "source.c++"
  0-18: "meta.preprocessor.c.include"
    1-8: "keyword.control.import.include.c" - Data: "include"
    9-18: "string.quoted.other.lt-gt.include.c"
      9-10: "punctuation.definition.string.begin.c" - Data: "<"
      17-18: "punctuation.definition.string.end.c" - Data: ">"
  19-42: "meta.preprocessor.c.include"
    20-27: "keyword.control.import.include.c" - Data: "include"
    28-42: "string.quoted.other.lt-gt.include.c"
      28-29: "punctuation.definition.string.begin.c" - Data: "<"
      41-42: "punctuation.definition.string.end.c" - Data: ">"
  43-67: "meta.preprocessor.c.include"
    44-51: "keyword.control.import.include.c" - Data: "include"
    52-67: "string.quoted.double.include.c"
      52-53: "punctuation.definition.string.begin.c" - Data: """
      66-67: "punctuation.definition.string.end.c" - Data: """
)").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()), result);

    QTextCursor cursor(&doc);
    cursor.setPosition(19, QTextCursor::MoveAnchor);
    cursor.setPosition(27, QTextCursor::KeepAnchor);
    // replace second #include with hoge
    cursor.insertText("hoge");
    highlighter.updateNode(19, 8, 4);
    QVERIFY(spy.wait());

    result = QString(R"r(
0-63: "source.c++"
  0-18: "meta.preprocessor.c.include"
    1-8: "keyword.control.import.include.c" - Data: "include"
    9-18: "string.quoted.other.lt-gt.include.c"
      9-10: "punctuation.definition.string.begin.c" - Data: "<"
      17-18: "punctuation.definition.string.end.c" - Data: ">"
  39-63: "meta.preprocessor.c.include"
    40-47: "keyword.control.import.include.c" - Data: "include"
    48-63: "string.quoted.double.include.c"
      48-49: "punctuation.definition.string.begin.c" - Data: """
      62-63: "punctuation.definition.string.end.c" - Data: """
)r").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()), result);
  }

  void replaceAllTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QString text = QString(R"(
#pragma once
#include <QDebug>

#include "App.h"
)").trimmed();
    QTextDocument doc(text);
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.c++", doc.toPlainText()));
    SyntaxHighlighter highlighter(&doc, std::move(parser), theme, font);
    QSignalSpy spy(&highlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    QFile output("testdata/highlighter_test/cppHighlightTestInput.res");
    QString result = QString(R"(
0-48: "source.c++"
  0-12: "meta.preprocessor.c"
    1-7: "keyword.control.import.c" - Data: "pragma"
  13-30: "meta.preprocessor.c.include"
    14-21: "keyword.control.import.include.c" - Data: "include"
    22-30: "string.quoted.other.lt-gt.include.c"
      22-23: "punctuation.definition.string.begin.c" - Data: "<"
      29-30: "punctuation.definition.string.end.c" - Data: ">"
  32-48: "meta.preprocessor.c.include"
    33-40: "keyword.control.import.include.c" - Data: "include"
    41-48: "string.quoted.double.include.c"
      41-42: "punctuation.definition.string.begin.c" - Data: """
      47-48: "punctuation.definition.string.end.c" - Data: """
)").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()), result);

    QTextCursor cursor(&doc);
    cursor.beginEditBlock();

    // replace first #include with hoge
    cursor.setPosition(13, QTextCursor::MoveAnchor);
    cursor.setPosition(21, QTextCursor::KeepAnchor);
    cursor.insertText("hoge");

    // replace second #include with hoge
    cursor.setPosition(28, QTextCursor::MoveAnchor);
    cursor.setPosition(36, QTextCursor::KeepAnchor);
    cursor.insertText("hoge");
    cursor.endEditBlock();

    highlighter.updateNode(13, 27, 19);
    QVERIFY(spy.wait());

    result = QString(R"r(
0-40: "source.c++"
  0-12: "meta.preprocessor.c"
    1-7: "keyword.control.import.c" - Data: "pragma"
  33-40: "string.quoted.double.c"
    33-34: "punctuation.definition.string.begin.c" - Data: """
    39-40: "punctuation.definition.string.end.c" - Data: """
)r").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()), result);
  }

  void undoReplaceAllTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QString text = QString(R"(
#pragma once
#include <QDebug>

#include "App.h"
)").trimmed();
    QTextDocument doc(text);
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.c++", doc.toPlainText()));
    SyntaxHighlighter highlighter(&doc, std::move(parser), theme, font);
    QSignalSpy spy(&highlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    QFile output("testdata/highlighter_test/cppHighlightTestInput.res");
    QString resultBeforeReplace = QString(R"(
0-48: "source.c++"
  0-12: "meta.preprocessor.c"
    1-7: "keyword.control.import.c" - Data: "pragma"
  13-30: "meta.preprocessor.c.include"
    14-21: "keyword.control.import.include.c" - Data: "include"
    22-30: "string.quoted.other.lt-gt.include.c"
      22-23: "punctuation.definition.string.begin.c" - Data: "<"
      29-30: "punctuation.definition.string.end.c" - Data: ">"
  32-48: "meta.preprocessor.c.include"
    33-40: "keyword.control.import.include.c" - Data: "include"
    41-48: "string.quoted.double.include.c"
      41-42: "punctuation.definition.string.begin.c" - Data: """
      47-48: "punctuation.definition.string.end.c" - Data: """
)").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()),
                                resultBeforeReplace);

    QTextCursor cursor(&doc);
    cursor.beginEditBlock();

    // replace first #include with hoge
    cursor.setPosition(13, QTextCursor::MoveAnchor);
    cursor.setPosition(21, QTextCursor::KeepAnchor);
    cursor.insertText("hoge");

    // replace second #include with hoge
    cursor.setPosition(28, QTextCursor::MoveAnchor);
    cursor.setPosition(36, QTextCursor::KeepAnchor);
    cursor.insertText("hoge");
    cursor.endEditBlock();

    highlighter.updateNode(13, 27, 19);
    QVERIFY(spy.wait());

    QString resultAfterReplace = QString(R"r(
0-40: "source.c++"
  0-12: "meta.preprocessor.c"
    1-7: "keyword.control.import.c" - Data: "pragma"
  33-40: "string.quoted.double.c"
    33-34: "punctuation.definition.string.begin.c" - Data: """
    39-40: "punctuation.definition.string.end.c" - Data: """
)r").trimmed();
    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()),
                                resultAfterReplace);

    // When
    doc.undo();
    highlighter.updateNode(13, 19, 27);
    QVERIFY(spy.wait());

    TestUtil::compareLineByLine(highlighter.rootNode().toString(doc.toPlainText()),
                                resultBeforeReplace);
  }
};

}  // namespace core

QTEST_MAIN(core::SyntaxHighlighterTest)
#include "SyntaxHighlighterTest.moc"
