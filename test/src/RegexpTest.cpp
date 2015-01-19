#include <QtTest/QtTest>

#include "Regexp.h"

class RegexpTest : public QObject {
  Q_OBJECT
 private slots:
  void compile() {
    Regexp* reg;
    for (int i = 0; i < 100; i++) {
      reg = Regexp::compile(R"((?x)
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
                                )?)");
      QVERIFY(reg);
    }

    reg = Regexp::compile(R"((?x)
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
                                )?)");
    QVERIFY(!reg);
  }

  void findStringSubmatchIndex() {
    Regexp* reg = Regexp::compile("(<\\?)\\s*([-_a-zA-Z0-9]+)");
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

  void escape() {
    QCOMPARE(Regexp::escape(R"(\$bc^)"), QString(R"(\$bc\^)"));
    QCOMPARE(Regexp::escape(R"(\\$bc^)"), QString(R"(\\\$bc\^)"));
    QCOMPARE(Regexp::escape(R"([]{}()|-*.\a?+^$# )"),
             QString(R"(\[\]\{\}\(\)\|\-\*\.\\a\?\+\^\$\#\ )"));
    QCOMPARE(Regexp::escape("\t\n\r\f\v"), QString("\\\t\\\n\\\r\\\f\\\v"));
  }
};

QTEST_MAIN(RegexpTest)
#include "RegexpTest.moc"
