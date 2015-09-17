#include <QtTest/QtTest>
#include <memory>

#include "Regexp.h"

namespace core {

class RegexpTest : public QObject {
  Q_OBJECT
 private slots:
  void compile() {
    std::unique_ptr<Regexp> reg;
    for (int i = 0; i < 100; i++) {
      reg.reset(Regexp::compile(R"((?x)
                                ^\s*\#\s*(define)\s+             # define
                                ((?<id>[a-zA-Z_][a-zA-Z0-9_]*))  # macro name
                                (?:                              # and optionally:
                                    (\()                         # an open parenthesis
                                        (
                                            \s* \g<id> \s*       # first argument
                                            ((,) \s* \g<id> \s*)*  # additional arguments
                                            (?:\.\.\.)?          # varargs ellipsis?
                                        )
                                    (\))                         # a close parenthesis
                                )?)"));
      QVERIFY(reg.get());
    }

    // This includes invalid \? pattern
    reg.reset(Regexp::compile(R"((?x)
                                ^\s*\#\s*(define)\s+             # define
                                ((\?<id>[a-zA-Z_][a-zA-Z0-9_]*))  # macro name
                                (?:                              # and optionally:
                                    (\()                         # an open parenthesis
                                        (
                                            \s* \g<id> \s*       # first argument
                                            ((,) \s* \g<id> \s*)*  # additional arguments
                                            (?:\.\.\.)?          # varargs ellipsis?
                                        )
                                    (\))                         # a close parenthesis
                                )?)"));
    QVERIFY(!reg.get());
  }

  void findStringSubmatchIndex() {
    Regexp* reg = Regexp::compile(R"((<\?)\s*([-_a-zA-Z0-9]+))");
    QString str = R"(<?xml version="1.0" encoding="UTF-8"?>)";
    QVector<int>* indices = reg->findStringSubmatchIndex(QStringRef(&str));
    QVERIFY(indices);
    QCOMPARE(indices->size(), 6);
    QCOMPARE(*indices, QVector<int>({0, 5, 0, 2, 2, 5}));

    // search fail
    str = "aaa";
    indices = reg->findStringSubmatchIndex(QStringRef(&str));
    QVERIFY(!indices);
  }

  void findStringSubmatchIndexInJapanese() {
    Regexp* reg = Regexp::compile(R"((いう(?:(a))?)\s*([あいうえお]+))");
    QString str = R"(あいうあいうえおかきくけこ)";
    QVector<int>* indices = reg->findStringSubmatchIndex(QStringRef(&str));
    QVERIFY(indices);
    QCOMPARE(indices->size(), 8);
    QCOMPARE(*indices, QVector<int>({1, 8, 1, 3, -1, -1, 3, 8}));

    // search fail
    str = "aaa";
    indices = reg->findStringSubmatchIndex(QStringRef(&str));
    QVERIFY(!indices);
  }

  void findStringSubmatchIndexBackward() {
    Regexp* reg = Regexp::compile("ab");
    QString str = "abcdabcd";
    QVector<int>* indices = reg->findStringSubmatchIndex(QStringRef(&str), true);
    QVERIFY(indices);
    QCOMPARE(indices->size(), 2);
    QCOMPARE(*indices, QVector<int>({4, 6}));

    // search fail
    str = "aaa";
    indices = reg->findStringSubmatchIndex(QStringRef(&str));
    QVERIFY(!indices);
  }

  void findJapaneseChar() {
    Regexp* reg = Regexp::compile("い");
    QString str = "あいうえお";
    QVector<int>* indices = reg->findStringSubmatchIndex(QStringRef(&str), true);
    QVERIFY(indices);
    QCOMPARE(indices->size(), 2);
    QCOMPARE(*indices, QVector<int>({1, 2}));
  }

  void findNotEmpty() {
    // Without ONIG_OPTION_FIND_NOT_EMPTY option, \b matches to an empty region in this test
    Regexp* reg = Regexp::compile(R"(\b\b)");
    QString str = "a";
    bool findNotEmpty = true;
    QVector<int>* indices = reg->findStringSubmatchIndex(str.midRef(0), false, findNotEmpty);
    QVERIFY(!indices);

    findNotEmpty = false;
    indices = reg->findStringSubmatchIndex(str.midRef(0), false, findNotEmpty);
    QVERIFY(indices);
    QCOMPARE(indices->size(), 2);
    QCOMPARE(*indices, QVector<int>({0, 0}));
  }

  void escape() {
    QCOMPARE(Regexp::escape(R"(\$bc^)"), QString(R"(\$bc\^)"));
    QCOMPARE(Regexp::escape(R"(\\$bc^)"), QString(R"(\\\$bc\^)"));
    QCOMPARE(Regexp::escape(R"([]{}()|-*.\a?+^$# )"),
             QString(R"(\[\]\{\}\(\)\|\-\*\.\\a\?\+\^\$\#\ )"));
    QCOMPARE(Regexp::escape("\t\n\r\f\v"), QString("\\\t\\\n\\\r\\\f\\\v"));
  }
};

}  // namespace core

QTEST_MAIN(core::RegexpTest)
#include "RegexpTest.moc"
