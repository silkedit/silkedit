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
    QString helloJson =
        u8R"({
"name": "hello_example",
"version": "0.1.0",
"description": "SilkEdit hello package example",
"main": "index.js",
"repository": ":silkedit/hello",
"author": "SilkEdit team",
"license": "MIT"
})";
    QJsonDocument doc = QJsonDocument::fromJson(helloJson.toUtf8());
    QVERIFY(!doc.isNull());
    Package pkg(QJsonValue(doc.object()));
    QStringList errors = pkg.validate();
    QCOMPARE(errors.size(), 0);
  }

  void tarballUrl() {
    QString shortUrlJson =
        u8R"({
"name": "hello_example",
"version": "0.1.0",
"description": "SilkEdit hello package example",
"main": "index.js",
"repository": "silkedit/hello",
"author": "SilkEdit team",
"license": "MIT"
})";
    QJsonDocument doc = QJsonDocument::fromJson(shortUrlJson.toUtf8());
    QVERIFY(!doc.isNull());
    Package pkg(QJsonValue(doc.object()));
    QCOMPARE(pkg.tarballUrl(),
             QString("https://github.com/silkedit/hello/tarball/0.1.0"));

    QString longUrlJson =
        u8R"({
"name": "hello_example",
"version": "0.1.0",
"description": "SilkEdit hello package example",
"main": "index.js",
"repository": {
  "type": "git",
  "url": "https:\/\/github.com/silkedit/hello.git"
},
"author": "SilkEdit team",
"license": "MIT"
})";
    doc = QJsonDocument::fromJson(longUrlJson.toUtf8());
    QVERIFY(!doc.isNull());
    pkg = Package(QJsonValue(doc.object()));
    QCOMPARE(pkg.tarballUrl(),
             QString("https://github.com/silkedit/hello/tarball/0.1.0"));
  }
};

}  // namespace core

QTEST_MAIN(core::PackageTest)
#include "PackageTest.moc"
