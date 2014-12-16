#include <QtTest/QtTest>
#include <QTextDocument>

#include "TmLanguage.h"
#include "SyntaxHighlighter.h"

class SyntaxHighlighterTest : public QObject {
  Q_OBJECT
 private slots:
  void scopeExtent();
};

void SyntaxHighlighterTest::scopeExtent()
{
  const QVector<QString> files({"testdata/Property List (XML).tmLanguage", "testdata/XML.plist"});

  foreach (QString fn, files) {
    QVERIFY(LanguageProvider::languageFromFile(fn));
  }

  QFile file("testdata/plist2.tmlang");
  QVERIFY(file.open(QIODevice::ReadOnly));

  QTextStream in(&file);
  QTextDocument* doc = new QTextDocument(in.readAll());
  LanguageParser* parser = LanguageParser::create("text.xml.plist", doc->toPlainText());
  auto highlighter = SyntaxHighlighter::create(doc, parser);
  Region region = highlighter->scopeExtent(10);
  QCOMPARE(region.a, 5);
  QCOMPARE(region.b, 13);
  QCOMPARE(highlighter->scopeName(10), QString("text.xml.plist meta.tag.preprocessor.xml entity.other.attribute-name.xml"));
}

QTEST_MAIN(SyntaxHighlighterTest)
#include "SyntaxHighlighterTest.moc"
