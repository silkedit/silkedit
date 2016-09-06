#include <algorithm>
#include <QtTest/QtTest>

#include "Theme.h"

namespace core {

class ThemeTest : public QObject {
  Q_OBJECT
 private slots:
  void loadTheme() {
    Theme* theme = Theme::loadTheme("testdata/Monokai.tmTheme");

    // name
    QCOMPARE(theme->name, QString("Monokai"));

    // gutterSettings
    QCOMPARE(theme->gutterSettings->size(), 3);
    QCOMPARE(theme->gutterSettings->value("background"), QColor("#49483E"));
    QCOMPARE(theme->gutterSettings->value("divider"), QColor("#75715E"));
    QCOMPARE(theme->gutterSettings->value("foreground"), QColor("#75715E"));

    // settings
    QCOMPARE(theme->scopeSettings.size(), 22);
    ScopeSetting* setting1 = theme->scopeSettings.at(0);
    QVERIFY(setting1->name.isEmpty());
    QVERIFY(setting1->scopeSelectors.isEmpty());
    QCOMPARE(setting1->colorSettings->size(), 6);
    QCOMPARE(setting1->colorSettings->value("background"), QColor("#272822"));

    ScopeSetting* setting2 = theme->scopeSettings.at(1);
    QCOMPARE(setting2->name, QString("Comment"));
    QCOMPARE(setting2->scopeSelectors, QStringList("comment"));
    QCOMPARE(setting2->colorSettings->size(), 1);
    QCOMPARE(setting2->colorSettings->value("foreground"), QColor("#75715E"));
  }

  void fontStyle() {
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
    QVERIFY(format);
    QCOMPARE(format->fontWeight(), (int)QFont::Bold);
    QCOMPARE(format->fontItalic(), true);
    QCOMPARE(format->fontUnderline(), true);
  }

  void getFormat() {
    Theme* theme = Theme::loadTheme("testdata/Monokai.tmTheme");
    auto format = theme->getFormat("entity.name.tag.localname.xml");
    QVERIFY(format);
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
    QVERIFY(format);
    QCOMPARE(format->foreground().color(), QColor("#AE81FF"));
    format = theme->getFormat("constant.other.entity.xml");
    QVERIFY(format);
    QCOMPARE(format->foreground().color(), QColor("#AE81FF"));
  }

  void skipEmptySettings() {
    //  <dict>
    //    <key>name</key>
    //    <string>Function argument</string>
    //    <key>scope</key>
    //    <string>variable.parameter</string>
    //    <key>settings</key>
    //    <dict/>
    //  </dict>
    //
    //  <dict>
    //    <key>name</key>
    //    <string>Library class/type</string>
    // ..skipping...
    //    <string>meta.preprocessor.c.include, meta.preprocessor.macro.c</string>
    //    <key>settings</key>
    //    <dict>
    //      <key>fontStyle</key>
    //      <string></string>
    //      <key>foreground</key>
    //      <string>#BB3700</string>
    //      </dict>
    //    </dict>
    Theme* theme = Theme::loadTheme("testdata/Solarized (Dark).tmTheme");
    auto format =
        theme->getFormat("source.c++ meta.preprocessor.macro.c variable.parameter.preprocessor.c");
    QVERIFY(format);
    QCOMPARE(format->foreground().color(), QColor("#BB3700"));
  }

  //  The winner is the scope selector which (in order of precedence):
  //  1. Match the element deepest down in the scope e.g. string wins over source.php when the scope
  //  is
  //  source.php string.quoted.
  //  2. Match most of the deepest element e.g. string.quoted wins over string.
  //  3. Rules 1 and 2 applied again to the scope selector when removing the deepest element (in the
  //  case of a tie), e.g. text source string wins over source string.
  //  http://manual.macromates.com/en/scope_selectors
  void rank() {
    //    int score = Theme::rank("variable.parameter", "variable.parameter.preprocessor.c");
    Rank rank("source.php", "source.php string.quoted");
    Rank rank2("string", "source.php string.quoted");
    Rank rank3("string.quoted", "source.php string.quoted");
    Rank rank4("source.php string", "source.php string.quoted");
    QVERIFY(rank2 > rank);
    QVERIFY(rank3 > rank2);
    QVERIFY(rank4 > rank);
    QVERIFY(rank4 > rank2);
    QVERIFY(rank4 < rank3);

    Rank rank10("text source string", "text source string");
    Rank rank11("text source", "text source string");
    Rank rank12("text", "text source string");
    QVERIFY(rank11 < rank10);
    QVERIFY(rank12 < rank11);
    QVERIFY(rank12 < rank10);

    Rank rank13("meta.property-value.css constant.numeric.css",
                "source.css meta.property-list.css meta.property-value.css constant.numeric.css "
                "keyword.other.unit.css");
    QVERIFY(!rank13.isInvalid());

    Rank rankInvalid("php", "source.php string.quoted");
    QVERIFY(rankInvalid.isInvalid());

    Rank rankInvalid2("text string", "text source string");
    QVERIFY(rankInvalid2.isInvalid());

    Rank rankInvalid3("string.quoted.foo", "source.php string.quoted");
    QVERIFY(rankInvalid3.isInvalid());

    Rank rankInvalid4("string.foo", "source.php string.quoted");
    QVERIFY(rankInvalid4.isInvalid());

    /*
     in Espresso Libre
     <key>name</key>
     <string>Embedded source</string>
     <key>scope</key>
     <string>text source, string.unquoted</string>

     this shouldn't match with source.c++
     */
    Rank rankInvalid5("text source", "source.c++ hoge");
    QVERIFY(rankInvalid5.isInvalid());

    // empty selector matches any scope but with the lowest rank
    Rank rankEmpty("", "source.php string.quoted");
    QVERIFY(rankEmpty.isEmpty());

    Rank rankValid("entity.name.tag", "entity.name.tag.localname.xml");
    QVERIFY(rankValid.isValid());
  }
};

}  // namespace core

QTEST_MAIN(core::ThemeTest)
#include "ThemeTest.moc"
