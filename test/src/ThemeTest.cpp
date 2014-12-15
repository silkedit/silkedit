#include <QtTest/QtTest>

#include "Theme.h"

namespace {
}

class ThemeTest : public QObject {
  Q_OBJECT
 private slots:
  void loadTheme();
};

void ThemeTest::loadTheme()
{
  Theme* theme = Theme::loadTheme("testdata/Monokai.tmTheme");

  // name
  QCOMPARE(theme->name, QString("Monokai"));

  // UUID
  QCOMPARE(theme->uuid, QUuid("D8D5E82E-3D5B-46B5-B38E-8C841C21347D"));

  // gutterSettings
  QCOMPARE(theme->gutterSettings->size(), 3);
  QCOMPARE(theme->gutterSettings->value("background"), QColor("#49483E"));
  QCOMPARE(theme->gutterSettings->value("divider"), QColor("#75715E"));
  QCOMPARE(theme->gutterSettings->value("foreground"), QColor("#75715E"));

  // settings
  QCOMPARE(theme->settings.size(), 22);
  ScopeSetting* setting1 = theme->settings.at(0);
  QVERIFY(setting1->name.isEmpty());
  QVERIFY(setting1->scope.isEmpty());
  QCOMPARE(setting1->settings->size(), 6);
  QCOMPARE(setting1->settings->value("background"), QColor("#272822"));

  ScopeSetting* setting2 = theme->settings.at(1);
  QCOMPARE(setting2->name, QString("Comment"));
  QCOMPARE(setting2->scope, QString("comment"));
  QCOMPARE(setting2->settings->size(), 1);
  QCOMPARE(setting2->settings->value("foreground"), QColor("#75715E"));
}

QTEST_MAIN(ThemeTest)
#include "ThemeTest.moc"
