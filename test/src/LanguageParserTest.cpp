#include <QtTest/QtTest>

#include "TmLanguage.h"

namespace {
  void compareLineByLine(const QString& str1, const QString& str2) {
    QStringList list1 = str1.split('\n');
    QStringList list2 = str2.split('\n');
    if (str1.size() != str2.size()) {
      qDebug() << str1;
    }
    QCOMPARE(list1.size(), list2.size());

    for (int i = 0; i < list1.size(); i++) {
      QCOMPARE(list1.at(i), list2.at(i));
    }
  }
}

class LanguageParserTest : public QObject {
  Q_OBJECT
 private slots:
  void test();
  void LanguageFromFile();
  void LanguageFromScope();
  void parseTmLanguage();
  void regexFind();
};

void LanguageParserTest::LanguageFromFile() {
  Language* lang = LanguageProvider::languageFromFile("testdata/C++.tmLanguage");

  // fileTypes
  QCOMPARE(lang->fileTypes.size(), 13);
  QCOMPARE(lang->fileTypes[0], QString("cpp"));

  // firstLineMatch
  QCOMPARE(lang->firstLineMatch, QString("-\\*- C\\+\\+ -\\*-"));

  // rootPattern
  Pattern* rootPattern = lang->rootPattern;
  QVERIFY(!rootPattern->patterns->isEmpty());
  foreach (Pattern* pat, *(rootPattern->patterns)) {
    QVERIFY(pat);
  }

  QCOMPARE(rootPattern->owner, lang);

  Pattern* includePattern = rootPattern->patterns->at(0);
  QCOMPARE(includePattern->include, QString("#special_block"));

  Pattern* matchPattern = rootPattern->patterns->at(2);
  QCOMPARE(matchPattern->match.re->pattern(), QString("\\b(friend|explicit|virtual)\\b"));
  QCOMPARE(matchPattern->name, QString("storage.modifier.c++"));

  Pattern* beginEndPattern = rootPattern->patterns->at(14);
  QVERIFY(!beginEndPattern->begin.re->pattern().isEmpty());
  QCOMPARE(beginEndPattern->beginCaptures.size(), 2);
  QVERIFY(!beginEndPattern->end.re->pattern().isEmpty());
  QCOMPARE(beginEndPattern->endCaptures.size(), 1);
  QCOMPARE(beginEndPattern->name, QString("meta.function.destructor.c++"));
  QCOMPARE(beginEndPattern->patterns->size(), 1);

  // repository
  QVERIFY(!lang->repository.isEmpty());
  Pattern* blockPat = lang->repository.value("block");
  QCOMPARE(blockPat->begin.re->pattern(), QString("\\{"));
  QCOMPARE(blockPat->end.re->pattern(), QString("\\}"));
  QCOMPARE(blockPat->name, QString("meta.block.c++"));
  QVector<Pattern*>* patterns = blockPat->patterns;
  QCOMPARE(patterns->size(), 2);
  Pattern* firstPat = patterns->at(0);
  QCOMPARE(firstPat->captures.size(), 2);
  QVERIFY(!firstPat->match.re->pattern().isEmpty());
  QCOMPARE(firstPat->name, QString("meta.function-call.c"));

  // scopeName
  QCOMPARE(lang->scopeName, QString("source.c++"));
}

void LanguageParserTest::LanguageFromScope()
{
  Language* lang = LanguageProvider::languageFromFile("testdata/C++.tmLanguage");
  Language* langFromScope = LanguageProvider::languageFromScope(lang->scopeName);
  QVERIFY(langFromScope);
  QVERIFY(!LanguageProvider::languageFromScope("missing scope"));
}

void LanguageParserTest::parseTmLanguage()
{
  const QVector<QString> files({"testdata/Property List (XML).tmLanguage", "testdata/XML.plist", "testdata/Go.tmLanguage"});

  foreach (QString fn, files) {
    QVERIFY(LanguageProvider::languageFromFile(fn));
  }

  QFile file("testdata/plist2.tmlang");
  QVERIFY(file.open(QIODevice::ReadOnly));

  QTextStream in(&file);
  LanguageParser* parser = LanguageParser::create("text.xml.plist", in.readAll());
  Node* root = parser->parse();

  QFile resFile("testdata/plist2.tmlang.res");
  QVERIFY(resFile.open(QIODevice::ReadOnly));

  QTextStream resIn(&resFile);
  compareLineByLine(root->toString(), resIn.readAll());
}

void LanguageParserTest::regexFind()
{
  Regex re("(<\\?)\\s*([-_a-zA-Z0-9]+)");
  MatchObject* mo = re.find(R"(<?xml version="1.0" encoding="UTF-8"?>)", 0);
  QCOMPARE(mo->size(), 6);
  QCOMPARE(*mo, QVector<int>({0, 5, 0, 2, 2, 5}));
}

void LanguageParserTest::test()
{
//  QVector<QString> vector(0);
//  vector.append("one");
//  vector.append("two");
//  vector.append("three");
//  QCOMPARE(vector.size(), 3);

//  QVector<QString> copied(vector);
//  QCOMPARE(copied.size(), 3);
  // vector: ["one", "two", "three"]
//  QRegularExpression re("(<\\?)\\s*([-_a-zA-Z0-9]+)");
//  QRegularExpressionMatch match = re.match("<?xml version=");
//  QVERIFY(match.hasMatch());

//  QVector<int> ret;
//  for (int i = 0; i < match.capturedLength(); i++) {
//    ret.append(match.capturedStart(i));
//    ret.append(match.capturedEnd(i));
//  }
//  QCOMPARE(match.captured(0), QString("<?xml"));
//  QCOMPARE(match.captured(1), QString("<?"));
//  QCOMPARE(match.captured(2), QString("xml"));
}

QTEST_MAIN(LanguageParserTest)
#include "LanguageParserTest.moc"
