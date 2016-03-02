#include <QtTest/QtTest>
#include <QTextDocument>

#include "LanguageParser.h"
#include "SyntaxHighlighter.h"
#include "Theme.h"
#include "TestUtil.h"

namespace core {

class SyntaxHighliterBenchmark : public QObject {
  Q_OBJECT
 private:
  Theme* theme = Theme::loadTheme("testdata/Solarized (Dark).tmTheme");
  QFont font = QFont("Helvetica");

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

  void syntaxHighlightTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QFile file("testdata/Benchmark_1007.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    QTextDocument* doc = new QTextDocument(in.readAll());
    const auto& text = doc->toPlainText();
    QTime startTime = QTime::currentTime();
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.c++", text));
    SyntaxHighlighter cppHighlighter(doc, std::move(parser), theme, font);
    QSignalSpy spy(&cppHighlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    int passed = startTime.msecsTo(QTime::currentTime());
    qDebug() << passed << "[ms]";
    QVERIFY(passed < 400);
  }

  void largeSyntaxHighlightTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }
    QFile file("testdata/Benchmark_12867.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    QTextDocument* doc = new QTextDocument(in.readAll());
    const auto& text = doc->toPlainText();
    QTime startTime = QTime::currentTime();
    std::unique_ptr<LanguageParser> parser(LanguageParser::create("source.c++", text));
    SyntaxHighlighter cppHighlighter(doc, std::move(parser), theme, font);
    QSignalSpy spy(&cppHighlighter, &SyntaxHighlighter::parseFinished);
    QVERIFY(spy.wait());

    int passed = startTime.msecsTo(QTime::currentTime());
    qDebug() << passed << "[ms]";
    QVERIFY(passed < 4000);
  }
};

}  // namespace core

QTEST_MAIN(core::SyntaxHighliterBenchmark)
#include "SyntaxHighlighterBenchmark.moc"
