#include <algorithm>
#include <QtTest/QtTest>

#include "Package.h"

namespace core {

class PackageTest : public QObject {
  Q_OBJECT
 private slots:
  void validateWithEmptyPackage() {
    QString emptyJson = R"({})";
    QJsonDocument doc = QJsonDocument::fromJson(emptyJson.toUtf8());
    QVERIFY(!doc.isNull());
    Package pkg(QJsonValue(doc.object()));
    QStringList errors = pkg.validate();
    QCOMPARE(errors.size(), 3);
  }

  void validateWithNonEmptyPackage() {
    QString emptyJson =
        u8R"({
"name": "hello_example",
"version": "0.1.0",
"description": "SilkEdit hello package example",
"main": "index.js",
"repository": ":silkedit/hello",
"author": "SilkEdit team",
"license": "MIT"
})";
    QJsonDocument doc = QJsonDocument::fromJson(emptyJson.toUtf8());
    QVERIFY(!doc.isNull());
    Package pkg(QJsonValue(doc.object()));
    QStringList errors = pkg.validate();
    QCOMPARE(errors.size(), 0);
  }

  void tarballUrl() {
    QString emptyJson =
        u8R"({
"name": "hello_example",
"version": "0.1.0",
"description": "SilkEdit hello package example",
"main": "index.js",
"repository": "silkedit/hello",
"author": "SilkEdit team",
"license": "MIT"
})";
    QJsonDocument doc = QJsonDocument::fromJson(emptyJson.toUtf8());
    QVERIFY(!doc.isNull());
    Package pkg(QJsonValue(doc.object()));
    QCOMPARE(pkg.tarballUrl(),
             QString("https://api.github.com/repos/silkedit/hello/tarball/0.1.0"));
  }
};

}  // namespace core

QTEST_MAIN(core::PackageTest)
#include "PackageTest.moc"
