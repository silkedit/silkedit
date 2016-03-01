#include <QtTest/QtTest>
#include <QTextBlock>

#include "Document.h"
#include "Regexp.h"
#include "LanguageParser.h"
#include "SyntaxHighlighter.h"

namespace core {

class DocumentTest : public QObject {
  Q_OBJECT

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
                                  "testdata/grammers/Rails/JavaScript (Rails).tmLanguage", "testdata/grammers/Makefile.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    // When
    Document jsErbDoc("hoge.old.js.erb", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    QSignalSpy spy(&jsErbDoc, &Document::parseFinished);
    QVERIFY(spy.wait());
    // Then
    QCOMPARE(jsErbDoc.language()->scopeName, QStringLiteral("source.js.rails source.js.jquery"));

    // When
    Document erbDoc("hoge.old.erb", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    QSignalSpy erbDocSpy(&erbDoc, &Document::parseFinished);
    QVERIFY(erbDocSpy.wait());
    // Then
    QCOMPARE(erbDoc.language()->scopeName, QStringLiteral("text.html.ruby"));

    // When
    Document plainTextDoc("hoge.invalid", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    QSignalSpy plainSpy(&plainTextDoc, &Document::parseFinished);
    QVERIFY(plainSpy.wait());
    // Then
    QCOMPARE(plainTextDoc.language()->scopeName, QStringLiteral("text.plain"));

    // When
    Document emptyPathDoc("", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    QSignalSpy emptySpy(&emptyPathDoc, &Document::parseFinished);
    QVERIFY(emptySpy.wait());
    // Then
    QCOMPARE(emptyPathDoc.language()->scopeName, QStringLiteral("text.plain"));

    // When
    Document makefile("hoge/foo/Makefile", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    QSignalSpy makeSpy(&makefile, &Document::parseFinished);
    QVERIFY(makeSpy.wait());
    // Then
    QCOMPARE(makefile.language()->scopeName, QStringLiteral("source.makefile"));
  }

  void setLanguage() {
    const QVector<QString> files({"testdata/grammers/Plain text.tmLanguage", "testdata/grammers/Rails/HTML (Rails).plist",
                                  "testdata/grammers/Rails/JavaScript (Rails).tmLanguage", "testdata/grammers/Makefile.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    // When
    Document jsErbDoc("hoge.old.js.erb", "", Encoding::defaultEncoding(), "", BOM::defaultBOM());
    QSignalSpy spy(&jsErbDoc, &Document::parseFinished);
    QVERIFY(spy.wait());

    // Then
    jsErbDoc.setLanguage("text.html.ruby");
    QVERIFY(spy.wait());
    QCOMPARE(jsErbDoc.language()->scopeName, QStringLiteral("text.html.ruby"));
  }
};

}  // namespace core

QTEST_MAIN(core::DocumentTest)
#include "DocumentTest.moc"
