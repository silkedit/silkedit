#include <QtTest/QtTest>
#include <QTextBlock>

#include "Document.h"
#include "Regexp.h"

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
};

}  // namespace core

QTEST_MAIN(core::DocumentTest)
#include "DocumentTest.moc"
