#include <string.h>
#include <stdio.h>
#include <string.h>
#include <QtTest/QtTest>
#include <QTextDocument>

#include "LanguageParser.h"
#include "TestUtil.h"

namespace core {

class LanguageParserTest : public QObject {
  Q_OBJECT
 private slots:
  void loadLanguage() {
    auto lang = LanguageProvider::loadLanguage("testdata/grammers/C++.tmLanguage");
    Q_ASSERT(lang);

    // fileTypes
    QCOMPARE(lang->fileTypes.size(), 13);
    QCOMPARE(lang->fileTypes[0], QString("cpp"));

    // firstLineMatch
    QCOMPARE(lang->firstLineMatch, QString("-\\*- C\\+\\+ -\\*-"));

    // rootPattern
    Pattern* rootPattern = lang->rootPattern.get();
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
    auto lang = LanguageProvider::loadLanguage("testdata/grammers/C++.tmLanguage");
    auto langFromScope = LanguageProvider::languageFromScope(lang->scopeName);
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
    const auto& text = in.readAll();
    LanguageParser* parser = LanguageParser::create("text.xml.plist", text);
    Q_ASSERT(parser);
    auto root = parser->parse();

    QFile resFile("testdata/plist.tmlang.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    TestUtil::compareLineByLine(root->toString(text), resIn.readAll());
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
    auto root = parser->parse();

    QString out = R"r(0-26: "source.c++"
  0-3: "storage.type.c" - Data: "int"
  3-25: "meta.function.c"
    3-4: "punctuation.whitespace.function.leading.c" - Data: " "
    4-8: "entity.name.function.c" - Data: "hoge"
    8-10: "meta.parens.c" - Data: "()"
    11-25: "meta.block.c"
      15-22: "constant.language.c++" - Data: "nullptr"
)r";
    TestUtil::compareLineByLine(root->toString(in), out);
  }

  //  // Test for $base when it doesn't have a parent syntax.
  void baseWithoutParentTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString in = R"(namespace {
  int hoge = "foo";
})";
    LanguageParser* parser = LanguageParser::create("source.c++", in);
    auto root = parser->parse();

    QString out = R"r(0-33: "source.c++"
  0-33: "meta.namespace-block.c++"
    0-9: "keyword.control.c++" - Data: "namespace"
    10-33: ""
      14-17: "storage.type.c" - Data: "int"
      25-30: "string.quoted.double.c"
        25-26: "punctuation.definition.string.begin.c" - Data: """
        29-30: "punctuation.definition.string.end.c" - Data: """)r";
    TestUtil::compareLineByLine(root->toString(in), out);
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
    auto root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-28: "source.c++"
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

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void cppIncludeTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(#include <string>)";
    LanguageParser* parser = LanguageParser::create("source.c++", text);
    auto root = parser->parse();

    QString result = R"r(0-17: "source.c++"
  0-17: "meta.preprocessor.c.include"
    1-8: "keyword.control.import.include.c" - Data: "include"
    9-17: "string.quoted.other.lt-gt.include.c"
      9-10: "punctuation.definition.string.begin.c" - Data: "<"
      16-17: "punctuation.definition.string.end.c" - Data: ">")r";

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void cppMultipleIncludesTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = QStringLiteral(R"(
#include <QDebug>

#include "App.h")"
).trimmed();

    LanguageParser* parser = LanguageParser::create("source.c++", text);
    auto root = parser->parse();

    QString result = QStringLiteral(R"r(
0-35: "source.c++"
  0-17: "meta.preprocessor.c.include"
    1-8: "keyword.control.import.include.c" - Data: "include"
    9-17: "string.quoted.other.lt-gt.include.c"
      9-10: "punctuation.definition.string.begin.c" - Data: "<"
      16-17: "punctuation.definition.string.end.c" - Data: ">"
  19-35: "meta.preprocessor.c.include"
    20-27: "keyword.control.import.include.c" - Data: "include"
    28-35: "string.quoted.double.include.c"
      28-29: "punctuation.definition.string.begin.c" - Data: """
      34-35: "punctuation.definition.string.end.c" - Data: """)r"
).trimmed();

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void cppRangeTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(class hoge {
  void foo();
};)";
    LanguageParser* parser = LanguageParser::create("source.c++", text);
    auto root = parser->parse();

    // ';' at the end of document is not included in any pattern, but root scope(source.c++)
    QString result = QStringLiteral(R"r(
0-29: "source.c++"
  0-28: "meta.class-struct-block.c++"
    0-5: "storage.type.c++" - Data: "class"
    6-10: "entity.name.type.c++" - Data: "hoge"
    11-28: ""
      11-12: "punctuation.definition.scope.c++" - Data: "{"
      15-19: "storage.type.c" - Data: "void"
      19-26: "meta.function.c"
        19-20: "punctuation.whitespace.function.leading.c" - Data: " "
        20-23: "entity.name.function.c" - Data: "foo"
        23-25: "meta.parens.c" - Data: "()"
      27-28: "punctuation.definition.invalid.c++" - Data: "}")r").trimmed();

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void cppFunctionTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/cppFunctionTest.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    const auto& text = in.readAll();

    LanguageParser* parser = LanguageParser::create("source.c++", text);
    auto root = parser->parse();

    QFile resFile("testdata/cppFunctionTest.cpp.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    TestUtil::compareLineByLine(root->toString(text), resIn.readAll());
  }

  void cppTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/cppTest.cpp");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    const auto& text = in.readAll();

    LanguageParser* parser = LanguageParser::create("source.c++", text);
    auto root = parser->parse();

    QFile resFile("testdata/cppTest.cpp.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    TestUtil::compareLineByLine(root->toString(text), resIn.readAll());
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
    auto root = parser->parse();
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

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void cppParensTest() {
    const QVector<QString> files(
        {"testdata/grammers/C.tmLanguage", "testdata/grammers/C++.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(Q(0, 0, 3))";

    LanguageParser* parser = LanguageParser::create("source.c++", text);
    auto root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-10: "source.c++"
  0-10: "meta.function.c"
    0-1: "entity.name.function.c" - Data: "Q"
    1-10: "meta.parens.c"
      2-3: "constant.numeric.c" - Data: "0"
      5-6: "constant.numeric.c" - Data: "0"
      8-9: "constant.numeric.c" - Data: "3")r";

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void javaPropertiesTest() {
    const QVector<QString> files({"testdata/grammers/JavaProperties.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/javaProperties.properties");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);
    const auto& text = in.readAll();

    LanguageParser* parser = LanguageParser::create("source.java-properties", text);
    auto root = parser->parse();

    QFile resFile("testdata/javaProperties.properties.res");
    QVERIFY(resFile.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream resIn(&resFile);
    TestUtil::compareLineByLine(root->toString(text), resIn.readAll());
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
    auto root = parser->parse();
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

    TestUtil::compareLineByLine(root->toString(text), result);
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
        {"testdata/grammers/Makefile.plist", "testdata/grammers/Shell-Unix-Bash.tmLanguage",
         "testdata/grammers/Ruby.plist", "testdata/grammers/Python.tmLanguage"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QFile file("testdata/Makefile");
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);

    LanguageParser* parser = LanguageParser::create("source.makefile", in.readAll());

    // When
    auto root = parser->parse();
    //    qDebug().noquote() << root->toString();

    // Then
    // at least no infinite loop
    // todo: support while and \G to parse Makefile grammer correctly
  }

  void yamlTest() {
    const QVector<QString> files({"testdata/grammers/YAML.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(- { key: ctrl+b, command: move_cursor_left, if: onMac })";

    LanguageParser* parser = LanguageParser::create("source.yaml", text);
    auto root = parser->parse();
//    qDebug().noquote() << root->toString();

    QString result = R"r(0-55: "source.yaml"
  0-2: "string.unquoted.yaml"
    0-1: "punctuation.definition.entry.yaml" - Data: "-"
    1-2: "string.unquoted.yaml" - Data: " "
  4-15: "string.unquoted.yaml"
    4-8: "entity.name.tag.yaml"
      7-8: "punctuation.separator.key-value.yaml" - Data: ":"
    9-15: "string.unquoted.yaml" - Data: "ctrl+b"
  17-42: "string.unquoted.yaml"
    17-25: "entity.name.tag.yaml"
      24-25: "punctuation.separator.key-value.yaml" - Data: ":"
    26-42: "string.unquoted.yaml" - Data: "move_cursor_left"
  44-54: "string.unquoted.yaml"
    44-47: "entity.name.tag.yaml"
      46-47: "punctuation.separator.key-value.yaml" - Data: ":"
    48-54: "string.unquoted.yaml" - Data: "onMac ")r";

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void patternThatMatchesMultiLineTest() {
    const QVector<QString> files({"testdata/grammers/YAML.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(menu:
- title: File)";

    /*
      This pattern matches the whole text because \s* matches a newline.
      In this case, force this pattern to match a single line

      <key>match</key>
      <string>(?:(?:(-\s*)?(\w+\s*(:)))|(-))\s*(?:((")[^"]*("))|((')[^']*('))|([^,{}&amp;#\[\]]+))\s*</string>
      <key>name</key>
      <string>string.unquoted.yaml</string>
     */

    LanguageParser* parser = LanguageParser::create("source.yaml", text);
    auto root = parser->parse();

    QString result = QString(R"r(
0-19: "source.yaml"
  0-6: "string.unquoted.yaml"
    0-5: "entity.name.tag.yaml"
      4-5: "punctuation.separator.key-value.yaml" - Data: ":"
    5-6: "string.unquoted.yaml" - Data: "
"
  6-19: "string.unquoted.yaml"
    6-8: "punctuation.definition.entry.yaml" - Data: "- "
    8-14: "entity.name.tag.yaml"
      13-14: "punctuation.separator.key-value.yaml" - Data: ":"
    15-19: "string.unquoted.yaml" - Data: "File"
)r").trimmed();

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void rubyHeredocTest() {
    const QVector<QString> files({"testdata/grammers/Ruby.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = u8R"(json = <<EOS # コーテーション無しでもOK
{
  "#{lang}": "#{RUBY}"
}
EOS)";

    LanguageParser* parser = LanguageParser::create("source.ruby", text);
    auto root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-59: "source.ruby"
  5-59: "string.unquoted.heredoc.ruby"
    5-12: "punctuation.definition.string.begin.ruby" - Data: "= <<EOS"
    34-41: "meta.embedded.line.ruby"
      34-36: "punctuation.section.embedded.begin.ruby" - Data: "#{"
      36-40: "source.ruby" - Data: "lang"
      40-41: "punctuation.section.embedded.end.ruby"
        40-41: "source.ruby" - Data: "}"
    45-52: "meta.embedded.line.ruby"
      45-47: "punctuation.section.embedded.begin.ruby" - Data: "#{"
      47-51: "variable.other.constant.ruby" - Data: "RUBY"
      47-51: "source.ruby" - Data: "RUBY"
      51-52: "punctuation.section.embedded.end.ruby"
        51-52: "source.ruby" - Data: "}"
    56-59: "punctuation.definition.string.end.ruby" - Data: "EOS")r";

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void rubyClassTest() {
    const QVector<QString> files({"testdata/grammers/Ruby.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(class Fred
  def initialize(v)
    @val = v
  end

  # Set it and get it.
  def set(v)
    @val = v
  end

  def get
    return @val
  end
end)";

    LanguageParser* parser = LanguageParser::create("source.ruby", text);
    auto root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-142: "source.ruby"
  0-10: "meta.class.ruby"
    0-5: "keyword.control.class.ruby" - Data: "class"
    6-10: "entity.name.type.class.ruby" - Data: "Fred"
  13-30: "meta.function.method.with-arguments.ruby"
    13-16: "keyword.control.def.ruby" - Data: "def"
    17-27: "entity.name.function.ruby" - Data: "initialize"
    27-28: "punctuation.definition.parameters.ruby" - Data: "("
    28-29: ""
      28-29: ""
        28-29: "variable.parameter.function.ruby" - Data: "v"
    29-30: "punctuation.definition.parameters.ruby" - Data: ")"
  35-39: "variable.other.readwrite.instance.ruby"
    35-36: "punctuation.definition.variable.ruby" - Data: "@"
  40-41: "keyword.operator.assignment.ruby" - Data: "="
  46-49: "keyword.control.ruby" - Data: "end"
  51-74: ""
    51-53: "punctuation.whitespace.comment.leading.ruby" - Data: "  "
    53-74: "comment.line.number-sign.ruby"
      53-54: "punctuation.definition.comment.ruby" - Data: "#"
  76-86: "meta.function.method.with-arguments.ruby"
    76-79: "keyword.control.def.ruby" - Data: "def"
    80-83: "entity.name.function.ruby" - Data: "set"
    83-84: "punctuation.definition.parameters.ruby" - Data: "("
    84-85: ""
      84-85: ""
        84-85: "variable.parameter.function.ruby" - Data: "v"
    85-86: "punctuation.definition.parameters.ruby" - Data: ")"
  91-95: "variable.other.readwrite.instance.ruby"
    91-92: "punctuation.definition.variable.ruby" - Data: "@"
  96-97: "keyword.operator.assignment.ruby" - Data: "="
  102-105: "keyword.control.ruby" - Data: "end"
  109-116: "meta.function.method.without-arguments.ruby"
    109-112: "keyword.control.def.ruby" - Data: "def"
    113-116: "entity.name.function.ruby" - Data: "get"
  121-127: "keyword.control.pseudo-method.ruby" - Data: "return"
  128-132: "variable.other.readwrite.instance.ruby"
    128-129: "punctuation.definition.variable.ruby" - Data: "@"
  135-138: "keyword.control.ruby" - Data: "end"
  139-142: "keyword.control.ruby" - Data: "end")r";

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void cssTest() {
    const QVector<QString> files({"testdata/grammers/CSS.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(a {
  hoge: 0;
})";

    LanguageParser* parser = LanguageParser::create("source.css", text);
    auto root = parser->parse();
    //    qDebug().noquote() << root->toString();

    QString result = R"r(0-16: "source.css"
  0-2: "meta.selector.css"
    0-1: "entity.name.tag.css" - Data: "a"
  2-16: ""
    2-16: "meta.property-list.css"
      2-3: "punctuation.section.property-list.begin.css" - Data: "{"
      6-10: "meta.property-name.css" - Data: "hoge"
      10-14: "meta.property-value.css"
        10-11: "punctuation.separator.key-value.css" - Data: ":"
        12-13: "constant.numeric.css" - Data: "0"
        13-14: "punctuation.terminator.rule.css" - Data: ";"
      15-16: "punctuation.section.property-list.end.css" - Data: "}")r";

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void cssCommentTest() {
    const QVector<QString> files({"testdata/grammers/CSS.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(/* a *
a)";

    LanguageParser* parser = LanguageParser::create("source.css", text);
    auto root = parser->parse();
    //    qDebug().noquote() << root->toString();

    // If we use raw string, it gives a compilation error...
    QString result = "0-8: \"source.css\"\n"
"  0-8: \"comment.block.css\"\n"
"    0-2: \"punctuation.definition.comment.css\" - Data: \"/*\"";

    TestUtil::compareLineByLine(root->toString(text), result);
  }

  void beginOfLine() {
    const QVector<QString> files({"testdata/grammers/CSS.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(
a {
  hoge: 0;
})";

    LanguageParser* parser = LanguageParser::create("source.css", text);
    QCOMPARE(parser->beginOfLine(-1), -1);
    QCOMPARE(parser->beginOfLine(0), 0);    // '\n'
    QCOMPARE(parser->beginOfLine(1), 1);    // 'a'
    QCOMPARE(parser->beginOfLine(2), 1);    // ' '
    QCOMPARE(parser->beginOfLine(3), 1);    // '{'
    QCOMPARE(parser->beginOfLine(4), 1);    // '\n'
    QCOMPARE(parser->beginOfLine(5), 5);    // ' '
    QCOMPARE(parser->beginOfLine(6), 5);    // ' '
    QCOMPARE(parser->beginOfLine(7), 5);    // 'h'
    QCOMPARE(parser->beginOfLine(14), 5);   // ';'
    QCOMPARE(parser->beginOfLine(15), 5);   // '\n'
    QCOMPARE(parser->beginOfLine(16), 16);  // '}'
    QCOMPARE(parser->beginOfLine(17), -1);  // end of document
  }

  void endOfLine() {
    const QVector<QString> files({"testdata/grammers/CSS.plist"});

    foreach (QString fn, files) { QVERIFY(LanguageProvider::loadLanguage(fn)); }

    QString text = R"(
a {
  hoge: 0;
})";

    LanguageParser* parser = LanguageParser::create("source.css", text);
    QCOMPARE(parser->endOfLine(-1), -1);
    QCOMPARE(parser->endOfLine(0), 0);     // '\n'
    QCOMPARE(parser->endOfLine(1), 4);     // 'a'
    QCOMPARE(parser->endOfLine(2), 4);     // ' '
    QCOMPARE(parser->endOfLine(3), 4);     // '{'
    QCOMPARE(parser->endOfLine(4), 4);     // '\n'
    QCOMPARE(parser->endOfLine(5), 15);    // ' '
    QCOMPARE(parser->endOfLine(6), 15);    // ' '
    QCOMPARE(parser->endOfLine(7), 15);    // 'h'
    QCOMPARE(parser->endOfLine(14), 15);   // ';'
    QCOMPARE(parser->endOfLine(15), 15);   // '\n'
    QCOMPARE(parser->endOfLine(16), 16);   // '}'
    QCOMPARE(parser->endOfLine(17), 16);   // end of document
    QCOMPARE(parser->endOfLine(100), 16);  // end of document
  }
};

}  // namespace core

QTEST_MAIN(core::LanguageParserTest)
#include "LanguageParserTest.moc"
