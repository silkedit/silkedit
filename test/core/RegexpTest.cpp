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
      reg = std::move(Regexp::compile(R"((?x)
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
    reg = std::move(Regexp::compile(R"((?x)
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
    auto reg = Regexp::compile(R"((<\?)\s*([-_a-zA-Z0-9]+))");
    QString str = R"(<?xml version="1.0" encoding="UTF-8"?>)";
    auto indices = reg->findStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 6);
    QCOMPARE(indices, QVector<int>({0, 5, 0, 2, 2, 5}));

    // search fail
    str = "aaa";
    indices = reg->findStringSubmatchIndex(str);
    QVERIFY(indices.isEmpty());
  }

  void findStringSubmatchIndexInRegion() {
    auto reg = Regexp::compile(R"(a(x*)b)");
    QString str = R"(-ab-axb-)";
    auto indices = reg->findStringSubmatchIndex(str, 2, 6);
    QVERIFY(indices.isEmpty());

    str = R"(-ab-axb-)";
    indices = reg->findStringSubmatchIndex(str, 2, 7);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 4);
    QCOMPARE(indices, QVector<int>({4, 7, 5, 6}));

    // test ^
    reg = Regexp::compile(R"(^)");
    str = R"(-ab-
axb-)";
    indices = reg->findStringSubmatchIndex(str, 2, 7);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices, QVector<int>({5, 5}));
  }

  void findAllStringSubmatchIndex() {
    auto reg = Regexp::compile(R"(a(x*)b)");
    QString str = R"(-ab-)";
    auto indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 1);
    QCOMPARE(indices[0], QVector<int>({1, 3, 2, 2}));

    str = R"(-axxb-)";
    indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 1);
    QCOMPARE(indices[0], QVector<int>({1, 5, 2, 4}));

    str = R"(-ab-axb-)";
    indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices[0], QVector<int>({1, 3, 2, 2}));
    QCOMPARE(indices[1], QVector<int>({4, 7, 5, 6}));

    str = R"(-axxb-ab-)";
    indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices[0], QVector<int>({1, 5, 2, 4}));
    QCOMPARE(indices[1], QVector<int>({6, 8, 7, 7}));

    reg = Regexp::compile(R"(aa)");
    str = R"(aaaaa)";
    indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices[0], QVector<int>({0, 2}));
    QCOMPARE(indices[1], QVector<int>({2, 4}));

    // search fail
    str = "-foo-";
    indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(indices.isEmpty());
  }

  void findStringSubmatchIndexInJapanese() {
    auto reg = Regexp::compile(u8R"((いう(?:(a))?)\s*([あいうえお]+))");
    QString str = u8R"(あいうあいうえおかきくけこ)";
    auto indices = reg->findStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 8);
    QCOMPARE(indices, QVector<int>({1, 8, 1, 3, -1, -1, 3, 8}));

    // search fail
    str = "aaa";
    indices = reg->findStringSubmatchIndex(str);
    QVERIFY(indices.isEmpty());
  }

  void findAllStringSubmatchIndexInJapanese() {
    auto reg = Regexp::compile(u8R"(あ(け*)い)");
    QString str = u8R"(-あい-あけい-)";
    auto indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices[0], QVector<int>({1, 3, 2, 2}));
    QCOMPARE(indices[1], QVector<int>({4, 7, 5, 6}));
  }

  void findStringSubmatchIndexForBOL() {
    auto reg = Regexp::compile(R"(^)");
    QString str = R"()";
    auto indices = reg->findStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices, QVector<int>({0, 0}));
  }

  void findAllStringSubmatchIndexForBOL() {
    auto reg = Regexp::compile(R"(^)");
    QString str = u8R"(あああ
いいい
ううう)";
    auto indices = reg->findAllStringSubmatchIndex(str);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 3);
    QCOMPARE(indices[0], QVector<int>({0, 0}));
    QCOMPARE(indices[1], QVector<int>({4, 4}));
    QCOMPARE(indices[2], QVector<int>({8, 8}));
  }

  void findAllStringSubmatchIndexForBOLInRegion() {
    auto reg = Regexp::compile(R"(^)");
    QString str = u8R"(あああ
いいい
ううう)";
    auto indices = reg->findAllStringSubmatchIndex(str, 1, 6);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 1);
    QCOMPARE(indices[0], QVector<int>({4, 4}));
  }

  void findStringSubmatchIndexBackward() {
    auto reg = Regexp::compile("ab");
    QString str = "abcdabcd";
    auto indices = reg->findStringSubmatchIndex(str, true);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices, QVector<int>({4, 6}));

    // search fail
    str = "aaa";
    indices = reg->findStringSubmatchIndex(str);
    QVERIFY(indices.isEmpty());
  }

  void findJapaneseChar() {
    auto reg = Regexp::compile(u8"い");
    QString str = u8"あいうえお";
    const auto indices = reg->findStringSubmatchIndex(str, true);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices, QVector<int>({1, 2}));
  }

  void findNotEmpty() {
    // Without ONIG_OPTION_FIND_NOT_EMPTY option, \b matches to an empty region in this test
    auto reg = Regexp::compile(R"(\b\b)");
    QString str = "a";
    bool findNotEmpty = true;
    auto indices = reg->findStringSubmatchIndex(str, 0, -1, false, findNotEmpty);
    QVERIFY(indices.isEmpty());

    findNotEmpty = false;
    indices = reg->findStringSubmatchIndex(str, 0, -1, false, findNotEmpty);
    QVERIFY(!indices.isEmpty());
    QCOMPARE(indices.size(), 2);
    QCOMPARE(indices, QVector<int>({0, 0}));
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
