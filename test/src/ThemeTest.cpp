#include <algorithm>
#include <QtTest/QtTest>

#include "Theme.h"

class ThemeTest : public QObject {
  Q_OBJECT
 private slots:
  void loadTheme();
  void fontStyle();
  void getFormat();
};

void ThemeTest::loadTheme() {
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
  QCOMPARE(theme->scopeSettings.size(), 22);
  ScopeSetting* setting1 = theme->scopeSettings.at(0);
  QVERIFY(setting1->name.isEmpty());
  QVERIFY(setting1->scopes.isEmpty());
  QCOMPARE(setting1->settings->size(), 6);
  QCOMPARE(setting1->settings->value("background"), QColor("#272822"));

  ScopeSetting* setting2 = theme->scopeSettings.at(1);
  QCOMPARE(setting2->name, QString("Comment"));
  QCOMPARE(setting2->scopes, QStringList("comment"));
  QCOMPARE(setting2->settings->size(), 1);
  QCOMPARE(setting2->settings->value("foreground"), QColor("#75715E"));
}

void ThemeTest::fontStyle() {
  Theme* theme = Theme::loadTheme("testdata/Test.tmTheme");

  // name
  QCOMPARE(theme->name, QString("All Hallow\'s Eve"));

  // fontStyle
  QVector<ScopeSetting*> settings = theme->scopeSettings;
  auto it = std::find_if(
      settings.constBegin(), settings.constEnd(),
      [this](const ScopeSetting* setting) { return setting->name == "Class inheritance"; });
  QVERIFY(it != settings.constEnd());
  ScopeSetting* setting = *it;
  QCOMPARE((int)setting->fontWeight, (int)QFont::Bold);
  QVERIFY(setting->isItalic);
  QVERIFY(setting->isUnderline);

  auto format = theme->getFormat("entity.other.inherited-class");
  QCOMPARE(format->fontWeight(), (int)QFont::Bold);
  QCOMPARE(format->fontItalic(), true);
  QCOMPARE(format->fontUnderline(), true);
}

void ThemeTest::getFormat() {
  Theme* theme = Theme::loadTheme("testdata/Monokai.tmTheme");
  auto format = theme->getFormat("entity.name.tag.localname.xml");
  QCOMPARE(format->foreground().color(), QColor("#F92672"));

  // scope has more than 1 selectors (13.4 Grouping).
  // http://manual.macromates.com/en/scope_selectors
  //  <dict>
  //          <key>name</key>
  //          <string>User-defined constant</string>
  //          <key>scope</key>
  //          <string>constant.character, constant.other</string>
  //          <key>settings</key>
  //          <dict>
  //                  <key>foreground</key>
  //                  <string>#AE81FF</string>
  //          </dict>
  //  </dict>
  format = theme->getFormat("constant.character.entity.xml");
  QCOMPARE(format->foreground().color(), QColor("#AE81FF"));
  format = theme->getFormat("constant.other.entity.xml");
  QCOMPARE(format->foreground().color(), QColor("#AE81FF"));
}

QTEST_MAIN(ThemeTest)
#include "ThemeTest.moc"
