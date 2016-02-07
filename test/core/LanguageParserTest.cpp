#include <string.h>
#include <stdio.h>
#include <string.h>
#include <QtTest/QtTest>
#include <QTextDocument>

#include "LanguageParser.h"

namespace {

void compareLineByLine(const QString& str1, const QString& str2) {
  QStringList list1 = str1.trimmed().split('\n');
  QStringList list2 = str2.trimmed().split('\n');
  if (str1.trimmed().size() != str2.trimmed().size()) {
    qDebug() << str1;
  }
  QCOMPARE(list1.size(), list2.size());

  for (int i = 0; i < list1.size(); i++) {
    QCOMPARE(list1.at(i).trimmed(), list2.at(i).trimmed());
  }
}
}

namespace core {

class LanguageParserTest : public QObject {
  Q_OBJECT
 private slots:
  void loadLanguage() {
    Language* lang = LanguageProvider::loadLanguage("testdata/C++.tmLanguage");
    Q_ASSERT(lang);

    // fileTypes
    QCOMPARE(lang->fileTypes.size(), 13);
    QCOMPARE(lang->fileTypes[0], QString("cpp"));

    // firstLineMatch
    QCOMPARE(lang->firstLineMatch, QString("-\\*- C\\+\\+ -\\*-"));

    // rootPattern
    Pattern* rootPattern = std::move(lang->rootPattern.get());
    QVERIFY(!rootPattern->patterns->isEmpty());
    foreach (Pattern* pat, *(rootPattern->patterns)) { QVERIFY(pat); }

    QCOMPARE(rootPattern->lang, lang);

    Pattern* includePattern = rootPattern->patterns->at(0);
    QCOMPARE(includePattern->include, QString("#special_block"));

    Pattern* matchPattern = rootPattern->patterns->at(2);
    QCOMPARE(matchPattern->match->pattern(), QString("\\b(friend|explicit|virtual)\\b"));
    QCOMPARE(matchPattern->name, QString("storage.modifier.c++"));

    Pattern* beginEndPattern = rootPattern->patterns->at(14);
    QVERIFY(!beginEndPattern->begin->pattern().isEmpty());
    QCOMPARE(beginEndPattern->beginCaptures.size(), 2);
    QVERIFY(!beginEndPattern->end->pattern().isEmpty());
    QCOMPARE(beginEndPattern->endCaptures.size(), 1);
    QCOMPARE(beginEndPattern->name, QString("meta.function.destructor.c++"));
    QCOMPARE(beginEndPattern->patterns->size(), 1);

    // repository
    QVERIFY(!lang->repository.empty());
    Pattern* blockPat = lang->repository.at("block").get();
    QCOMPARE(blockPat->begin->pattern(), QString("\\{"));
    QCOMPARE(blockPat->end->pattern(), QString("\\}"));
    QCOMPARE(blockPat->name, QString("meta.block.c++"));
    QVector<Pattern*>* patterns = blockPat->patterns.get();
    QCOMPARE(patterns->size(), 2);
    Pattern* firstPat = patterns->at(0);
    QCOMPARE(firstPat->captures.size(), 2);
    QVERIFY(!firstPat->match->pattern().isEmpty());
    QCOMPARE(firstPat->name, QString("meta.function-call.c"));

    // scopeName
    QCOMPARE(lang->scopeName, QString("source.c++"));
  }

  void LanguageFromScope() {
    Language* lang = LanguageProvider::loadLanguage("testdata/C++.tmLanguage");
    Language* langFromScope = LanguageProvider::languageFromScope(lang->scopeName);
    QVERIFY(langFromScope);
    QVERIFY(!LanguageProvider::languageFromScope("missing scope"));
  }

  void parseTmLanguage() {
    const QVector<QString> files({"testdata/C++.tmLanguage",
                                  "testdata/C.tmLanguage",
                                  "testdata/Property List (XML).tmLanguage",
                                  "testdata/XML.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/plist.tmlang");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    LanguageParser* parser = LanguageParser::create("text.xml.plist", in.readAll());
    Node* root = parser->parse();

    QFile resFile("testdata/plist.tmlang.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    compareLineByLine(root->toString(), resIn.readAll());
  }

  // Test for $base when it has a parent syntax.
  // When source.c++ includes source.c, "include $base" in source.c means including source.c++
  void baseWithParentTest() {
    const QVector<QString> files({"testdata/C.tmLanguage", "testdata/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString in = R"(int hoge() {
  nullptr;
}
)";
    LanguageParser* parser = LanguageParser::create("source.c++", in);
    Node* root = parser->parse();

    QString out = R"r(0-25: "source.c++"
  0-3: "storage.type.c" - Data: "int"
  3-25: "meta.function.c"
    3-4: "punctuation.whitespace.function.leading.c" - Data: " "
    4-8: "entity.name.function.c" - Data: "hoge"
    8-10: "meta.parens.c" - Data: "()"
    11-25: "meta.block.c"
      15-22: "constant.language.c++" - Data: "nullptr"
)r";
    compareLineByLine(root->toString(), out);
  }

  //  // Test for $base when it doesn't have a parent syntax.
  void baseWithoutParentTest() {
    const QVector<QString> files({"testdata/C.tmLanguage", "testdata/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString in = R"(namespace {
  int hoge = "foo";
})";
    LanguageParser* parser = LanguageParser::create("source.c++", in);
    Node* root = parser->parse();

    QString out = R"r(0-33: "source.c++"
  0-33: "meta.namespace-block.c++"
    0-9: "keyword.control.c++" - Data: "namespace"
    10-33: ""
      14-17: "storage.type.c" - Data: "int"
      25-30: "string.quoted.double.c"
        25-26: "punctuation.definition.string.begin.c" - Data: """
        29-30: "punctuation.definition.string.end.c" - Data: """)r";
    compareLineByLine(root->toString(), out);
  }

  // Test to check if $ end pattern doesn't match every line
  //
  // <key>end</key>
  // <string>(?=(?://|/\*))|$</string>
  // <key>name</key>
  // <string>meta.preprocessor.macro.c</string>
  void endOfLineMatchTest() {
    const QVector<QString> files({"testdata/C.tmLanguage", "testdata/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(#define foo(a) \
    bar(a)
)";
    LanguageParser* parser = LanguageParser::create("source.c++", text);
    Node* root = parser->parse();

    QString result = R"r(0-27: "source.c++"
  0-17: "meta.preprocessor.macro.c"
    1-7: "keyword.control.import.define.c" - Data: "define"
    8-11: "entity.name.function.preprocessor.c" - Data: "foo"
    11-12: "punctuation.definition.parameters.c" - Data: "("
    12-13: "variable.parameter.preprocessor.c" - Data: "a"
    13-14: "punctuation.definition.parameters.c" - Data: ")"
    15-17: "punctuation.separator.continuation.c" - Data: "\
"
  17-27: "meta.function.c"
    17-21: "punctuation.whitespace.function.leading.c" - Data: "    "
    21-24: "entity.name.function.c" - Data: "bar")r";

    compareLineByLine(root->toString(), result);
  }

  void cppIncludeTest() {
    const QVector<QString> files({"testdata/C.tmLanguage", "testdata/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(#include <string>)";
    LanguageParser* parser = LanguageParser::create("source.c++", text);
    Node* root = parser->parse();

    QString result = R"r(0-17: "source.c++"
  0-17: "meta.preprocessor.c.include"
    1-8: "keyword.control.import.include.c" - Data: "include"
    9-17: "string.quoted.other.lt-gt.include.c"
      9-10: "punctuation.definition.string.begin.c" - Data: "<"
      16-17: "punctuation.definition.string.end.c" - Data: ">")r";

    compareLineByLine(root->toString(), result);
  }

  void cppFunctionTest() {
    const QVector<QString> files({"testdata/C.tmLanguage", "testdata/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/cppFunctionTest.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);

    LanguageParser* parser = LanguageParser::create("source.c++", in.readAll());
    Node* root = parser->parse();

    QFile resFile("testdata/cppFunctionTest.cpp.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    compareLineByLine(root->toString(), resIn.readAll());
  }

  void hasBackReference() {
    QVERIFY(Regex::hasBackReference(R"(\1)"));
    QVERIFY(Regex::hasBackReference(R"(\7)"));
    QVERIFY(Regex::hasBackReference(R"(\10)"));
    QVERIFY(Regex::hasBackReference(R"(\35)"));
    QVERIFY(!Regex::hasBackReference(R"(\\10)"));
    QVERIFY(Regex::hasBackReference(R"(\\\10)"));
    QVERIFY(!Regex::hasBackReference(R"(\\\\10)"));
  }
};

}  // namespace core

QTEST_MAIN(core::LanguageParserTest)
#include "LanguageParserTest.moc"
