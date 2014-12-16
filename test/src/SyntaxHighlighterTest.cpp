#include <QtTest/QtTest>
#include <QTextDocument>

#include "TmLanguage.h"
#include "SyntaxHighlighter.h"

class SyntaxHighlighterTest : public QObject {
  Q_OBJECT
 private slots:
  void scopeExtent();
};

void SyntaxHighlighterTest::scopeExtent() {
  const QVector<QString> files({"testdata/Property List (XML).tmLanguage", "testdata/XML.plist"});

  foreach (QString fn, files) { QVERIFY(LanguageProvider::languageFromFile(fn)); }

  QFile file("testdata/plist2.tmlang");
  QVERIFY(file.open(QIODevice::ReadOnly));

  QTextStream in(&file);
  QTextDocument* doc = new QTextDocument(in.readAll());
  LanguageParser* plistParser = LanguageParser::create("text.xml.plist", doc->toPlainText());
  auto plistHighlighter = SyntaxHighlighter::create(doc, plistParser);
  Region region = plistHighlighter->scopeExtent(10);
  QCOMPARE(region.a, 5);
  QCOMPARE(region.b, 13);
  QCOMPARE(plistHighlighter->scopeName(10),
           QString("text.xml.plist meta.tag.preprocessor.xml entity.other.attribute-name.xml"));

  region = plistHighlighter->scopeExtent(14);
  QCOMPARE(region.a, 14);
  QCOMPARE(region.b, 15);
  QCOMPARE(plistHighlighter->scopeName(14),
           QString(
               "text.xml.plist meta.tag.preprocessor.xml string.quoted.double.xml "
               "punctuation.definition.string.begin.xml"));

  // XML

  LanguageParser* xmlParser = LanguageParser::create("text.xml", doc->toPlainText());
  auto xmlHighlighter = SyntaxHighlighter::create(doc, xmlParser);

  // Node at 148 has an empty name like this

  // 142-163: "meta.tag.xml"
  //   142-143: "punctuation.definition.tag.xml" - Data: "<"
  //   143-148: "entity.name.tag.localname.xml" - Data: "plist"
  //   148-157: ""
  //     149-156: "entity.other.attribute-name.localname.xml" - Data: "version"
  region = xmlHighlighter->scopeExtent(148);
  QCOMPARE(region.a, 148);
  QCOMPARE(region.b, 157);
  QCOMPARE(xmlHighlighter->scopeName(148), QString("text.xml meta.tag.xml"));

  region = xmlHighlighter->scopeExtent(149);
  QCOMPARE(region.a, 149);
  QCOMPARE(region.b, 156);
  QCOMPARE(xmlHighlighter->scopeName(149), QString("text.xml meta.tag.xml entity.other.attribute-name.localname.xml"));
}

QTEST_MAIN(SyntaxHighlighterTest)
#include "SyntaxHighlighterTest.moc"
