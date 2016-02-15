#include <QtTest/QtTest>
#include <QTextBlock>

#include "Document.h"
#include "Regexp.h"
#include "LanguageParser.h"

namespace core {

class DocumentTest : public QObject {
  Q_OBJECT

 private slots:
  void findAll() {
    QString text =
        R"(aaa
bbb
ccc)";
    Document doc;
    doc.setPlainText(text);

    auto cursor = QTextCursor(&doc);
    cursor.movePosition(QTextCursor::End);
    auto regex = Regexp::compile("^");
    auto regions = doc.findAll(regex.get(), 0, cursor.position());
    Q_ASSERT(!regions.isEmpty());
    QCOMPARE(regions.size(), 3);
    QCOMPARE(regions[0], Region(0, 0));
    QCOMPARE(regions[1], Region(4, 4));
    QCOMPARE(regions[2], Region(8, 8));
  }

  void selectGrammerFromExtension() {
    const QVector<QString> files({"testdata/grammers/Plain text.tmLanguage", "testdata/grammers/Rails/HTML (Rails).plist",
                                  "testdata/grammers/Rails/JavaScript (Rails).tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    // When
    Document jsErbDoc("hoge.old.js.erb", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    // Then
    QCOMPARE(jsErbDoc.language()->scopeName, QStringLiteral("source.js.rails source.js.jquery"));

    // When
    Document erbDoc("hoge.old.erb", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    // Then
    QCOMPARE(erbDoc.language()->scopeName, QStringLiteral("text.html.ruby"));

    // When
    Document plainTextDoc("hoge.invalid", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    // Then
    QCOMPARE(plainTextDoc.language()->scopeName, QStringLiteral("text.plain"));

    // When
    Document emptyPathDoc("", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    // Then
    QCOMPARE(emptyPathDoc.language()->scopeName, QStringLiteral("text.plain"));
  }
};

}  // namespace core

QTEST_MAIN(core::DocumentTest)
#include "DocumentTest.moc"
