#include <QDebug>
#include "Theme.h"
#include "PListParser.h"
#include "Util.h"

namespace core {

namespace {

const QString nameStr = "name";
const QString scopeStr = "scope";
const QString settingsStr = "settings";
const QString foregroundStr = "foreground";
const QString backgroundStr = "background";
const int brightnessThresholdWhite = 180;
const int brightnessThresholdGray = 125;
const int brightnessThresholdBlack = 60;
const QString& verticalScrollBarBaseStyle = QStringLiteral(R"r(
QScrollBar:vertical {
  border-left: 0px;
  background: %1;
  width: 8px;
  margin: 0px 0px 0px 0px;
}

QScrollBar::handle:vertical {
  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop: 0  %2, stop: 0.5 %2,  stop:1 %2);
  min-height: 0px;
  border-radius: 4px;
}

QScrollBar::add-line:vertical {
  height: 0px;
}

QScrollBar::sub-line:vertical {
  height: 0px;
}
)r");

const QString& horizontalScrollBarBaseStyle = QStringLiteral(R"r(
QScrollBar:horizontal {
  border-left: 0px;
  background: %1;
  height: 8px;
  margin: 0px 0px 0px 0px;
}

QScrollBar::handle:horizontal {
  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop: 0  %2, stop: 0.5 %2,  stop:1 %2);
  min-width: 0px;
  border-radius: 4px;
}

QScrollBar::add-line:horizontal {
  width: 0px;
}

QScrollBar::sub-line:horizontal {
  width: 0px;
}
)r");

const QString& scrollBarColor = QStringLiteral("#C1C1C1");

// Change RGBA color code to ARGB color code
//   thTheme in package defines color ilke below.
//     #RRGGBBAA
//   But QColor::setNamedColor is below (http://doc.qt.io/qt-5/qcolor.html#setNamedColor)
//   #AARRGGBB (Since 5.2)
QString convertColorCodeToARGB(QString colorCode) {
  QString colorRgb = colorCode.mid(1, 6);
  QString colorAlpha = "FF";
  if (colorCode.length() == 9) {
    colorAlpha = colorCode.right(2);
  }
  return QString("#%1%2").arg(colorAlpha, colorRgb);
}

void parseSettings(ColorSettings* settings,
                   QFont::Weight* fontWeight,
                   bool* isItalic,
                   bool* isUnderline,
                   QVariant var) {
  if (!var.canConvert<QVariantMap>()) {
    return;
  }

  QVariantMap map = var.toMap();
  QMapIterator<QString, QVariant> iter(map);
  while (iter.hasNext()) {
    iter.next();
    QString key = iter.key();
    if (key == "fontStyle") {
      QStringList styles = iter.value().toString().split(' ');
      foreach (const QString& style, styles) {
        if (style == "bold") {
          *fontWeight = QFont::Bold;
        } else if (style == "italic") {
          *isItalic = true;
        } else if (style == "underline") {
          *isUnderline = true;
        }
      }
    } else {
      QString colorCode = iter.value().toString();
      QColor color;

      color.setNamedColor(convertColorCodeToARGB(colorCode));
      if (color.isValid()) {
        (*settings)[key] = color;
      }
    }
  }
}

void parseSettings(ColorSettings* settings, ColorSettings defaultColors) {
  QMapIterator<QString, QColor> i(defaultColors);
  while (i.hasNext()) {
    i.next();
    if (i.value().isValid()) {
      (*settings)[i.key()] = i.value();
    }
  }
}

QColor changeColorBrightness(QColor const color,
                             int value = 10,
                             int threshold = brightnessThresholdGray) {
  QColor newColor;

  // 0 is black; 255 is as far from black as possible.
  if (color.value() < threshold) {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), qMin(color.value() + value, 255));
  } else {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), qMax(color.value() - value, 0));
  }
  return newColor;
}

QColor changeColorBrightnessDarker(QColor const color, int value = 10) {
  return QColor::fromHsv(color.hue(), color.saturation(), qMax(color.value() - value, 0));
}

QColor getAppropriateGray(QColor const color, bool reverse = false, int alpha = 255) {
  QColor newColor;
  int brightness = color.value();
  if (reverse) {
    brightness = 255 - brightness;
  }
  // use material color
  //  - http://www.materialui.co/colors
  if (brightness < brightnessThresholdBlack) {
    newColor.setRgb(224, 224, 224, alpha);  // 300
  } else if (brightness < brightnessThresholdGray) {
    newColor.setRgb(158, 158, 158, alpha);  // 500
  } else if (brightness < brightnessThresholdWhite) {
    newColor.setRgb(97, 97, 97, alpha);  // 700
  } else {
    newColor.setRgb(33, 33, 33, alpha);  // 900
  }
  return newColor;
}

QColor getHoverColor(QColor const color, int alpha = 50) {
  QColor newColor;
  int brightness = color.value();
  if (brightness < brightnessThresholdGray) {
    newColor.setRgb(158, 158, 158, alpha);
  } else {
    newColor.setRgb(0, 0, 0, alpha);
  }
  return newColor;
}

QColor getSelectedBorderColor() {
  // use material color
  //  - http://www.materialui.co/colors
  return QColor::fromRgb(41, 98, 255);  // Blue A700
}

ScopeSetting* toScopeSetting(QVariant var) {
  if (!var.canConvert<QVariantMap>()) {
    return nullptr;
  }

  QVariantMap map = var.toMap();
  ScopeSetting* scopeSetting = new ScopeSetting();

  // name
  if (map.contains(nameStr)) {
    scopeSetting->name = map.value(nameStr).toString();
  }

  // scope
  if (map.contains(scopeStr)) {
    QStringList scopeList = map.value(scopeStr).toString().split(',');
    for (int i = 0; i < scopeList.length(); i++) {
      scopeList[i] = scopeList[i].trimmed();
    }
    scopeSetting->scopeSelectors = scopeList;
  }

  // settings
  if (map.contains(settingsStr)) {
    scopeSetting->colorSettings.reset(new ColorSettings());
    parseSettings(scopeSetting->colorSettings.get(), &(scopeSetting->fontWeight),
                  &(scopeSetting->isItalic), &(scopeSetting->isUnderline), map.value(settingsStr));
  }

  return scopeSetting;
}
}

Theme* Theme::loadTheme(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning("unable to open a file");
    return nullptr;
  }

  QVariant root = PListParser::parsePList(&file);
  if (!root.canConvert<QVariantMap>()) {
    qWarning("root is not dict");
    return nullptr;
  }

  Theme* theme = new Theme();
  QVariantMap rootMap = root.toMap();

  // name
  if (rootMap.contains(nameStr)) {
    theme->name = rootMap.value(nameStr).toString();
  }

  // create base theme
  if (rootMap.contains(settingsStr)) {
    QVariantList settingList = rootMap.value(settingsStr).toList();
    foreach (const QVariant& var, settingList) {
      // This is the base color in the following process,
      theme->scopeSettings.append(toScopeSetting(var));
    }
  }

  // text edit settings (TextEdit)
  theme->textEditSettings.reset(new ColorSettings());
  parseSettings(theme->textEditSettings.get(), createTextEditSettingsColors(theme));

  // gutter settings (LineNumberArea)
  const QString gutterSettingsStr = "gutterSettings";
  theme->gutterSettings.reset(new ColorSettings());

  if (rootMap.contains(gutterSettingsStr)) {
    parseSettings(theme->gutterSettings.get(), &(theme->gutterFontWeight), &(theme->isGutterItalic),
                  &(theme->isGutterUnderline), rootMap.value(gutterSettingsStr));
  } else {
    parseSettings(theme->gutterSettings.get(), createGutterSettingsColors(theme));
  }

  // status bar settings(StatusBar)
  theme->statusBarSettings.reset(new ColorSettings());
  parseSettings(theme->statusBarSettings.get(), createStatusBarSettingsColors(theme));

  // tab bar settings(TabBar)
  theme->tabBarSettings.reset(new ColorSettings());
  parseSettings(theme->tabBarSettings.get(), createTabBarSettingsColors(theme));

  // tab view settings(TabView)
  theme->tabViewSettings.reset(new ColorSettings());
  parseSettings(theme->tabViewSettings.get(), createTabViewSettingsColors(theme));

  // project tree view settings(ProjectTreeView)
  theme->projectTreeViewSettings.reset(new ColorSettings());
  parseSettings(theme->projectTreeViewSettings.get(), createProjectTreeViewSettingsColors(theme));

  // window settings(window)
  theme->windowSettings.reset(new ColorSettings());
  parseSettings(theme->windowSettings.get(), createWindowSettingsColors(theme));

  // package tool bar settings(PackageToolBar)
  theme->packageToolBarSettings.reset(new ColorSettings());
  parseSettings(theme->packageToolBarSettings.get(), createPackageToolBarSettingsColors(theme));

  // Find Replace View settings(FindReplaceView)
  theme->findReplaceViewSettings.reset(new ColorSettings());
  parseSettings(theme->findReplaceViewSettings.get(), createFindReplaceViewSettingsColors(theme));

  // Console settings(Console)
  theme->consoleSettings.reset(new ColorSettings());
  parseSettings(theme->consoleSettings.get(), createConsleSettingsColors(theme));

  return theme;
}

bool Theme::isDarkTheme() const {
  bool ret = false;
  if (scopeSettings.isEmpty()) {
    return ret;
  }

  ColorSettings* baseColorSettings = scopeSettings.first()->colorSettings.get();
  if (baseColorSettings->isEmpty()) {
    return ret;
  }

  if (!baseColorSettings->contains("background")) {
    return ret;
  }

  QColor backgroundColor = baseColorSettings->value("background");
  if (backgroundColor.value() < brightnessThresholdGray) {
    ret = true;
  }
  return ret;
}

boost::optional<QFont> Theme::font() {
  return m_font;
}

ColorSettings Theme::createTextEditSettingsColors(const Theme* theme) {
  ColorSettings defaultColors;
  QColor backgroundColor = QColor(Qt::gray);
  QColor foregroundColor = QColor(Qt::black);
  QColor lineHighlight = QColor(Qt::gray);
  QColor selectionBackgroundColor = QColor(Qt::gray);
  QColor selectionForegroundColor = QColor(Qt::black);

  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* baseColorSettings = theme->scopeSettings.first()->colorSettings.get();
    if (!baseColorSettings->isEmpty()) {
      if (baseColorSettings->contains("foreground")) {
        foregroundColor = baseColorSettings->value("foreground");
      }

      if (baseColorSettings->contains("background")) {
        backgroundColor = baseColorSettings->value("background");
      }

      if (baseColorSettings->contains("lineHighlight")) {
        lineHighlight = baseColorSettings->value("lineHighlight");
      }

      if (baseColorSettings->contains("selection")) {
        selectionBackgroundColor = baseColorSettings->value("selection");
      } else if (baseColorSettings->contains("selectionBackground")) {
        selectionBackgroundColor = baseColorSettings->value("selectionBackground");
      }

      // for selection foreground color, we use foreground color if selectionForeground is not
      // found.
      // The reason is that Qt ignores syntax highlighted color for a selected text and sets
      // selection
      // foreground color something.
      // Sometimes it becomes the color hard to see. We use foreground color instead to prevent it.
      // https://bugreports.qt.io/browse/QTBUG-1344?jql=project%20%3D%20QTBUG%20AND%20text%20~%20%22QTextEdit%20selection%20color%22
      if (baseColorSettings->contains("selectionForeground")) {
        selectionForegroundColor = baseColorSettings->value("selectionForeground");
      } else {
        selectionForegroundColor = foregroundColor;
      }
    }
  }

  return defaultColors = {{"background", backgroundColor},
                          {"foreground", foregroundColor},
                          {"lineHighlight", lineHighlight},
                          {"selectionBackground", selectionBackgroundColor},
                          {"selectionForeground", selectionForegroundColor}};
}

ColorSettings Theme::createGutterSettingsColors(const Theme* theme) {
  ColorSettings defaultColors;
  QColor backgroundColor = QColor(Qt::gray);
  QColor foregroundColor = QColor(Qt::black);

  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* baseColorSettings = theme->scopeSettings.first()->colorSettings.get();

    if (!baseColorSettings->isEmpty()) {
      if (baseColorSettings->contains("background")) {
        backgroundColor = changeColorBrightness(baseColorSettings->value("background"));
      }

      if (baseColorSettings->contains("foreground")) {
        foregroundColor = changeColorBrightness(baseColorSettings->value("foreground"));
      }
    }
  }

  return defaultColors = {{"background", backgroundColor}, {"foreground", foregroundColor}};
}

ColorSettings Theme::createStatusBarSettingsColors(const Theme* theme) {
  ColorSettings defaultColors;
  QColor backgroundColor = QColor(Qt::Window);
  QColor foregroundColor = QColor(Qt::black);

  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* baseColorSettings = theme->scopeSettings.first()->colorSettings.get();

    if (baseColorSettings->contains("background")) {
      backgroundColor = changeColorBrightnessDarker(baseColorSettings->value("background"), 20);
    }
    if (baseColorSettings->contains("foreground")) {
      foregroundColor = changeColorBrightness(baseColorSettings->value("foreground"));
    }
  }

  return defaultColors = {{"background", backgroundColor}, {"foreground", foregroundColor}};
}

ColorSettings Theme::createTabViewSettingsColors(const Theme* theme) {
  return createStatusBarSettingsColors(theme);
}

ColorSettings Theme::createTabBarSettingsColors(const Theme* theme) {
  ColorSettings defaultColors;
  QColor backgroundColor = QColor(Qt::Window);
  QColor foregroundColor = QColor(Qt::gray);
  QColor selectedColor = QColor(Qt::black);
  QColor selectedBorderColor = QColor(Qt::white);

  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* baseColorSettings = theme->scopeSettings.first()->colorSettings.get();

    if (!baseColorSettings->isEmpty()) {
      if (baseColorSettings->contains("background")) {
        selectedColor = baseColorSettings->value("background");
        selectedBorderColor = getSelectedBorderColor();
        backgroundColor = changeColorBrightness(baseColorSettings->value("background"));
      }

      if (baseColorSettings->contains("foreground")) {
        foregroundColor = changeColorBrightness(baseColorSettings->value("foreground"));
      }
    }
  }
  return defaultColors = {{"background", backgroundColor},
                          {"foreground", foregroundColor},
                          {"selected", selectedColor},
                          {"selectedBorder", selectedBorderColor}};
}

ColorSettings Theme::createProjectTreeViewSettingsColors(const Theme* theme) {
  ColorSettings textEditViewSettingsColors = createTextEditSettingsColors(theme);
  ColorSettings defaultColors = createStatusBarSettingsColors(theme);
  defaultColors["selectionForeground"] = getAppropriateGray(defaultColors["background"], true);
  defaultColors["selectionBackground"] =
      getAppropriateGray(defaultColors["background"], false, 130);

  return defaultColors;
}

ColorSettings Theme::createWindowSettingsColors(const Theme* theme) {
  ColorSettings defaultColors;
  QColor backgroundColor = QColor(33, 33, 33);     // Gray 900
  QColor foregroundColor = QColor(117, 117, 117);  // Gray 600

  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* baseColorSettings = theme->scopeSettings.first()->colorSettings.get();

    if (baseColorSettings->contains("background")) {
      backgroundColor = baseColorSettings->value("background");
      foregroundColor = getAppropriateGray(baseColorSettings->value("background"));
    }
  }
  return defaultColors = {{"background", backgroundColor}, {"foreground", foregroundColor}};
}

ColorSettings Theme::createPackageToolBarSettingsColors(const Theme* theme) {
  return createStatusBarSettingsColors(theme);
}

ColorSettings Theme::createFindReplaceViewSettingsColors(const Theme* theme) {
  ColorSettings defaultColors = createStatusBarSettingsColors(theme);
  defaultColors["buttonCheckedBackgroundColor"] =
      getAppropriateGray(defaultColors.value("background"));
  defaultColors["focusColor"] = getSelectedBorderColor();
  defaultColors["hoverColor"] = getHoverColor(defaultColors.value("background"));
  defaultColors["pressedColor"] = getHoverColor(defaultColors.value("background"), 150);

  return defaultColors;
}

ColorSettings Theme::createConsleSettingsColors(const Theme* theme) {
  return createStatusBarSettingsColors(theme);
}

// Return the rank of scopeSelector for scope
// Return the rank greater than or equal to 0 if scopeSelector matches scope
// Return -1 if it doesn't match scope
int Theme::rank(const QString& scopeSelector, const QString& scope) {
  // An empty scope selector will match all scopes but with the lowest possible rank
  // http://manual.macromates.com/en/scope_selectors
  if (scopeSelector.isEmpty()) {
    return 0;
  }

  int score = -1;
  for (int i = 0; i < qMin(scopeSelector.length(), scope.length()); i++) {
    if (scopeSelector[i] == scope[i]) {
      score++;
    }
  }

  if (score >= 0) {
    score++;
  }
  return score;
}

QVector<ScopeSetting*> Theme::getMatchedSettings(const QString& scope) {
  std::vector<std::tuple<Rank, ScopeSetting*>> settingsWithRank;
  foreach (ScopeSetting* setting, scopeSettings) {
    foreach (const QString& selector, setting->scopeSelectors) {
      Rank rank(selector, scope);
      if (rank.isValid()) {
        settingsWithRank.push_back(std::make_pair(rank, setting));
      }
    }
  }

  std::sort(
      settingsWithRank.begin(), settingsWithRank.end(),
      [](const std::tuple<Rank, ScopeSetting*>& s1, const std::tuple<Rank, ScopeSetting*>& s2) {
        return std::get<0>(s1) > std::get<0>(s2);
      });

  QVector<ScopeSetting*> matchedSettings;
  for (std::tuple<Rank, ScopeSetting*>& setting : settingsWithRank) {
    matchedSettings.append(std::get<1>(setting));
  }

  return matchedSettings;
}

void Theme::setFont(const QFont& font) {
  m_font = font;

  // Update cache
  // Don't set QFont directly, or it overwrites font weight and italics properties.
  for (auto& pair : m_cachedFormats) {
    pair.second->setFontFamily(font.family());
    pair.second->setFontPointSize(font.pointSizeF());
  }
}

QString Theme::textEditVerticalScrollBarStyle() const {
  return verticalScrollBarBaseStyle.arg(Util::qcolorForStyleSheet(
                                            textEditSettings->value(QStringLiteral("background"))))
      .arg(scrollBarColor);
}

QString Theme::projectTreeViewVerticalScrollBarStyle() const {
  return verticalScrollBarBaseStyle.arg(Util::qcolorForStyleSheet(projectTreeViewSettings->value(
                                            QStringLiteral("background"))))
      .arg(scrollBarColor);
}

QString Theme::textEditHorizontalScrollBarStyle() const {
  return horizontalScrollBarBaseStyle.arg(Util::qcolorForStyleSheet(textEditSettings->value(
                                              QStringLiteral("background"))))
      .arg(scrollBarColor);
}

QString Theme::projectTreeViewHorizontalScrollBarStyle() const {
  return horizontalScrollBarBaseStyle.arg(Util::qcolorForStyleSheet(projectTreeViewSettings->value(
                                              QStringLiteral("background"))))
      .arg(scrollBarColor);
}

QTextCharFormat* Theme::getFormat(const QString& scope) {
  if (scopeSettings.isEmpty())
    return nullptr;

  // check cache
  if (m_cachedFormats.count(scope) != 0) {
    return m_cachedFormats.at(scope).get();
  }

  std::unique_ptr<QTextCharFormat> format(new QTextCharFormat());

  if (m_font) {
    format->setFontFamily((*m_font).family());
    format->setFontPointSize((*m_font).pointSizeF());
  }

  QVector<ScopeSetting*> matchedSettings = getMatchedSettings(scope);
  if (!matchedSettings.isEmpty()) {
    // foreground
    foreach (ScopeSetting* setting, matchedSettings) {
      if (setting->colorSettings->contains(foregroundStr)) {
        const QColor& fg = setting->colorSettings->value(foregroundStr);
        Q_ASSERT(fg.isValid());
        format->setForeground(fg);
        break;
      }
    }

    // background
    foreach (ScopeSetting* setting, matchedSettings) {
      if (setting->colorSettings->contains(backgroundStr)) {
        const QColor& bg = setting->colorSettings->value(backgroundStr);
        Q_ASSERT(bg.isValid());
        format->setBackground(bg);
        break;
      }
    }

    // font style
    foreach (ScopeSetting* setting, matchedSettings) {
      if (setting->hasFontStyle()) {
        format->setFontWeight(setting->fontWeight);
        format->setFontItalic(setting->isItalic);
        format->setFontUnderline(setting->isUnderline);
        break;
      }
    }
  }

  auto result = format.get();
  m_cachedFormats.insert(std::make_pair(scope, std::move(format)));
  return result;
}

Rank::Rank(const QString& scopeSelector, const QString& scope) {
  if (scopeSelector.isEmpty()) {
    m_state = State::Empty;
  } else {
    QVector<QStringRef> selectors = scopeSelector.splitRef(QStringLiteral(" "));
    QVector<QStringRef> scopes = scope.splitRef(QStringLiteral(" "));
    QVector<int> scores(scopes.size(), 0);

    if (selectors.size() > scopes.size()) {
      m_state = State::Invalid;
      return;
    }

    int selectorsIndex = 0, scopesIndex = 0;
    bool lastMatched = false;
    for (; selectorsIndex < selectors.size() && scopesIndex < scopes.size(); scopesIndex++) {
      int rank = calcRank(selectors[selectorsIndex], scopes[scopesIndex]);
      if (rank > 0) {
        scores[scopesIndex] = rank;
        selectorsIndex++;
        lastMatched = true;
      } else if (lastMatched) {
        m_state = State::Invalid;
        return;
      }
    }

    // if selectorsIndex is less than selectors.size(),
    // it means not all the selectors matches
    if (selectorsIndex < selectors.size()) {
      m_state = State::Invalid;
      return;
    }

    int zeroCount = 0;
    foreach (int score, scores) {
      if (score == 0) {
        zeroCount++;
      }
    }
    if (zeroCount == scores.size()) {
      m_state = State::Invalid;
    } else {
      m_state = State::Valid;
      m_scores = scores;
    }
  }
}

bool Rank::operator>(const Rank& r2) const {
  if (this->isInvalid() || r2.isInvalid()) {
    return false;
  }

  if (this->isEmpty() && r2.isEmpty()) {
    return true;
  } else if (this->isEmpty() && !r2.isEmpty()) {
    return false;
  } else if (r2.isEmpty()) {
    return true;
  }

  if (this->m_scores.size() != r2.m_scores.size()) {
    throw std::runtime_error("can't compare. m_scores size don't match");
  }

  int i = this->m_scores.size() - 1;
  while (i >= 0) {
    if (this->m_scores[i] > r2.m_scores[i]) {
      return true;
    } else if (this->m_scores[i] < r2.m_scores[i]) {
      return false;
    }
    i--;
  }

  // r1 equals to r2
  return false;
}

bool Rank::operator<(const Rank& r2) const {
  if (this->isInvalid() || r2.isInvalid()) {
    return false;
  }

  if (this->isEmpty() && r2.isEmpty()) {
    return true;
  } else if (this->isEmpty() && !r2.isEmpty()) {
    return true;
  } else if (r2.isEmpty()) {
    return false;
  }

  if (this->m_scores.size() != r2.m_scores.size()) {
    throw std::runtime_error("can't compare. m_scores size don't match");
  }

  int i = this->m_scores.size() - 1;
  while (i >= 0) {
    if (this->m_scores[i] < r2.m_scores[i]) {
      return true;
    } else if (this->m_scores[i] > r2.m_scores[i]) {
      return false;
    }
    i--;
  }

  // r1 equals to r2
  return false;
}

// singleScopeSelector and singleScope must NOT have a space
int Rank::calcRank(const QStringRef& singleScopeSelector, const QStringRef& singleScope) {
  if (singleScopeSelector.size() > singleScope.size()) {
    return 0;
  }

  int score = 0;
  int from = 0;
  int dotIndex = singleScope.indexOf('.', from);

  while (dotIndex != -1 && from < singleScopeSelector.size()) {
    if (singleScope.mid(from, dotIndex - from) == singleScopeSelector.mid(from, dotIndex - from)) {
      score++;
    } else {
      return 0;
    }
    from = dotIndex + 1;
    dotIndex = singleScope.indexOf('.', from);
  }

  if (from < singleScope.size() && from < singleScopeSelector.size()) {
    if (singleScope.mid(from) == singleScopeSelector.mid(from)) {
      score++;
    } else {
      return 0;
    }
  }

  return score;
}

bool ScopeSetting::hasFontStyle() {
  return fontWeight != QFont::Normal || isItalic || isUnderline;
}

}  // namespace core
