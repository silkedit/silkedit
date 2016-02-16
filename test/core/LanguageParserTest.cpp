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
    qDebug().noquote() << str1;
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
    Language* lang = LanguageProvider::loadLanguage("testdata/grammers/C++.tmLanguage");
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
    QVERIFY(!rootPattern->repository.empty());
    Pattern* blockPat = rootPattern->repository.at("block").get();
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
    Language* lang = LanguageProvider::loadLanguage("testdata/grammers/C++.tmLanguage");
    Language* langFromScope = LanguageProvider::languageFromScope(lang->scopeName);
    QVERIFY(langFromScope);
    QVERIFY(!LanguageProvider::languageFromScope("missing scope"));
  }

  void parseTmLanguage() {
    const QVector<QString> files(
        {"testdata/grammers/C++.tmLanguage", "testdata/grammers/C.tmLanguage",
         "testdata/grammers/Property List (XML).tmLanguage", "testdata/grammers/XML.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/plist.tmlang");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    LanguageParser* parser = LanguageParser::create("text.xml.plist", in.readAll());
    std::unique_ptr<Node> root = parser->parse();

    QFile resFile("testdata/plist.tmlang.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    compareLineByLine(root->toString(), resIn.readAll());
  }

  // Test for $base when it has a parent syntax.
  // When source.c++ includes source.c, "include $base" in source.c means including source.c++
  void baseWithParentTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString in = R"(int hoge() {
  nullptr;
}
)";
    LanguageParser* parser = LanguageParser::create("source.c++", in);
    std::unique_ptr<Node> root = parser->parse();

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
    const QVector<QString> files({"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString in = R"(namespace {
  int hoge = "foo";
})";
    LanguageParser* parser = LanguageParser::create("source.c++", in);
    std::unique_ptr<Node> root = parser->parse();

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
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(#define foo(a) \
    bar(a)
)";
    LanguageParser* parser = LanguageParser::create("source.c++", text);
    std::unique_ptr<Node> root = parser->parse();
    //    qDebug().noquote() << root->toString();

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
    21-24: "entity.name.function.c" - Data: "bar"
    24-27: "meta.parens.c" - Data: "(a)")r";

    compareLineByLine(root->toString(), result);
  }

  void cppIncludeTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(#include <string>)";
    LanguageParser* parser = LanguageParser::create("source.c++", text);
    std::unique_ptr<Node> root = parser->parse();

    QString result = R"r(0-17: "source.c++"
  0-17: "meta.preprocessor.c.include"
    1-8: "keyword.control.import.include.c" - Data: "include"
    9-17: "string.quoted.other.lt-gt.include.c"
      9-10: "punctuation.definition.string.begin.c" - Data: "<"
      16-17: "punctuation.definition.string.end.c" - Data: ">")r";

    compareLineByLine(root->toString(), result);
  }

  void cppFunctionTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/cppFunctionTest.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);

    LanguageParser* parser = LanguageParser::create("source.c++", in.readAll());
    std::unique_ptr<Node> root = parser->parse();

    QFile resFile("testdata/cppFunctionTest.cpp.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    compareLineByLine(root->toString(), resIn.readAll());
  }

  void cppTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/cppTest.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);

    LanguageParser* parser = LanguageParser::create("source.c++", in.readAll());
    std::unique_ptr<Node> root = parser->parse();

    QFile resFile("testdata/cppTest.cpp.res");
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

  void contentNameTest() {
    const QVector<QString> files({"testdata/grammers/JavaProperties.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(web = http\:/\/en.wikipedia.org/
language = English)";
    LanguageParser* parser = LanguageParser::create("source.java-properties", text);
    std::unique_ptr<Node> root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-51: "source.java-properties"
  0-33: "meta.key-value.java-properties"
    0-3: "support.constant.java-properties" - Data: "web"
    4-5: "punctuation.separator.key-value.java-properties" - Data: "="
    6-32: "string.unquoted.java-properties" - Data: "http\:/\/en.wikipedia.org/"
  33-51: "meta.key-value.java-properties"
    33-41: "support.constant.java-properties" - Data: "language"
    42-43: "punctuation.separator.key-value.java-properties" - Data: "="
    44-51: "string.unquoted.java-properties" - Data: "English")r";

    compareLineByLine(root->toString(), result);
  }

  void cppParensTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(Q(0, 0, 3))";

    LanguageParser* parser = LanguageParser::create("source.c++", text);
    std::unique_ptr<Node> root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-10: "source.c++"
  0-10: "meta.function.c"
    0-1: "entity.name.function.c" - Data: "Q"
    1-10: "meta.parens.c"
      2-3: "constant.numeric.c" - Data: "0"
      5-6: "constant.numeric.c" - Data: "0"
      8-9: "constant.numeric.c" - Data: "3")r";

    compareLineByLine(root->toString(), result);
  }

  void javaPropertiesTest() {
    const QVector<QString> files({"testdata/grammers/JavaProperties.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/javaProperties.properties");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);

    LanguageParser* parser = LanguageParser::create("source.java-properties", in.readAll());
    std::unique_ptr<Node> root = parser->parse();

    QFile resFile("testdata/javaProperties.properties.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    compareLineByLine(root->toString(), resIn.readAll());
  }

  void sqlErbTest() {
    const QVector<QString> files(
        {"testdata/grammers/SQL.plist", "testdata/grammers/Rails/SQL (Rails).plist",
         "testdata/grammers/Rails/Ruby on Rails.plist", "testdata/grammers/Ruby.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(# app/sql_queries/get_player_by_email.sql.erb
SELECT *
FROM players
WHERE email = <%= quote @email %>)";

    LanguageParser* parser = LanguageParser::create("source.sql.ruby", text);
    std::unique_ptr<Node> root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-101: "source.sql.ruby"
  0-46: ""
    0-46: "comment.line.number-sign.sql"
      0-1: "punctuation.definition.comment.sql" - Data: "#"
  46-52: "keyword.other.DML.sql" - Data: "SELECT"
  53-54: "keyword.operator.star.sql" - Data: "*"
  55-59: "keyword.other.DML.sql" - Data: "FROM"
  68-73: "keyword.other.DML.sql" - Data: "WHERE"
  80-81: "keyword.operator.comparison.sql" - Data: "="
  82-101: "source.ruby.rails.embedded.sql"
    92-98: "variable.other.readwrite.instance.ruby"
      92-93: "punctuation.definition.variable.ruby" - Data: "@")r";

    compareLineByLine(root->toString(), result);
  }

  // Makefile.plist includes \G pattern
  /*
          <key>begin</key>
          <string>\G</string>
          <key>end</key>
          <string>^</string>
          <key>name</key>
          <string>meta.scope.prerequisites.makefile</string>
   */
  void makefileTest() {
    const QVector<QString> files(
        {"testdata/grammers/Makefile.plist", "testdata/grammers/Shell-Unix-Bash.tmLanguage", "testdata/grammers/Ruby.plist", "testdata/grammers/Python.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/Makefile");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);

    LanguageParser* parser = LanguageParser::create("source.makefile", in.readAll());

    // When
    std::unique_ptr<Node> root = parser->parse();
    //    qDebug().noquote() << root->toString();

    // Then
    // at least no infinite loop
    // todo: support while and \G to parse Makefile grammer correctly
  }
};

}  // namespace core

QTEST_MAIN(core::LanguageParserTest)
#include "LanguageParserTest.moc"
