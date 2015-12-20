﻿#include <QDebug>
#include "Theme.h"
#include "PListParser.h"

namespace core {

namespace {

const QString nameStr = "name";
const QString scopeStr = "scope";
const QString settingsStr = "settings";
const QString foregroundStr = "foreground";
const QString backgroundStr = "background";
const int brightnessThresholdWhite = 180;
const int brightnessThresholdGrey = 125;
const int brightnessThresholdBlack = 60;

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
      QColor color(iter.value().toString());
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
                             int threshold = brightnessThresholdGrey) {
  QColor newColor;

  // 0 is black; 255 is as far from black as possible.
  if (color.value() < threshold) {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), color.value() + value);
  } else {
    newColor = QColor::fromHsv(color.hue(), color.saturation(), color.value() - value);
  }
  return newColor;
}

QColor changeColorBrightnessDarker(QColor const color, int value = 10) {
  QColor newColor;
  newColor = QColor::fromHsv(color.hue(), color.saturation(), color.value() - value);
  return newColor;
}

QColor getAppropriateGrey(QColor const color, bool reverse = false) {
  QColor newColor;
  int brightness = color.value();
  if(reverse) {
    brightness = 255 - brightness;
  }
  // use material color
  //  - http://www.materialui.co/colors
  if (brightness < brightnessThresholdBlack) {
    newColor.setRgb(224, 224, 224);  // 300
  } else if (brightness < brightnessThresholdGrey) {
    newColor.setRgb(158, 158, 158);  // 500
  } else if (brightness < brightnessThresholdWhite) {
    newColor.setRgb(97, 97, 97);  // 700
  } else {
    newColor.setRgb(33, 33, 33);  // 900
  }
  return newColor;
}

QColor getSelectedTabBorderColor(QColor const color) {
  QColor newColor;
  int brightness = color.value();
  // use material color
  //  - http://www.materialui.co/colors
  if (brightness < brightnessThresholdGrey) {
    newColor.setRgb(130, 177, 255);  // Blue A100
  } else {
    newColor.setRgb(41, 98, 255);  // Blue A700
  }
  return newColor;
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

  // main editor settings (TextEditView)
  if (rootMap.contains(settingsStr)) {
    QVariantList settingList = rootMap.value(settingsStr).toList();
    foreach (const QVariant& var, settingList) {
      // This is the base color In the following process,
      theme->scopeSettings.append(toScopeSetting(var));
    }
  }

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

  return theme;
}

bool Theme::isDarkTheme() {
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

  QColor backgroundColor = baseColorSettings->value("background").name();
  if (backgroundColor.value() < brightnessThresholdGrey) {
    ret = true;
  }
  return ret;
}

ColorSettings Theme::createGutterSettingsColors(const Theme* theme) {
  ColorSettings defaultColors;
  QColor backgroundColor = QColor(Qt::gray);
  QColor foregroundColor = QColor(Qt::black);

  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* baseColorSettings = theme->scopeSettings.first()->colorSettings.get();

    if (!baseColorSettings->isEmpty()) {
      if (baseColorSettings->contains("background")) {
        backgroundColor = changeColorBrightness(baseColorSettings->value("background").name());
      }

      if (baseColorSettings->contains("foreground")) {
        foregroundColor = changeColorBrightness(baseColorSettings->value("foreground").name());
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
      backgroundColor =
          changeColorBrightnessDarker(baseColorSettings->value("background").name(), 20);
    }
    if (baseColorSettings->contains("foreground")) {
      foregroundColor = changeColorBrightness(baseColorSettings->value("foreground").name());
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
        selectedColor = baseColorSettings->value("background").name();
        selectedBorderColor =
            getSelectedTabBorderColor(baseColorSettings->value("background").name());
        backgroundColor = changeColorBrightness(baseColorSettings->value("background").name());
      }

      if (baseColorSettings->contains("foreground")) {
        foregroundColor = changeColorBrightness(baseColorSettings->value("foreground").name());
      }
    }
  }
  return defaultColors = {{"background", backgroundColor},
                          {"foreground", foregroundColor},
                          {"selected", selectedColor},
                          {"selectedBorder", selectedBorderColor}};
}

ColorSettings Theme::createProjectTreeViewSettingsColors(const Theme* theme) {
  return createStatusBarSettingsColors(theme);
}

ColorSettings Theme::createWindowSettingsColors(const Theme* theme) {
  ColorSettings defaultColors;
  QColor backgroundColor = QColor(33, 33, 33);     // Grey 900
  QColor foregroundColor = QColor(117, 117, 117);  // Grey 600

  if (!theme->scopeSettings.isEmpty()) {
    ColorSettings* baseColorSettings = theme->scopeSettings.first()->colorSettings.get();

    if (baseColorSettings->contains("background")) {
      backgroundColor = baseColorSettings->value("background").name();
      foregroundColor = getAppropriateGrey(baseColorSettings->value("background").name());
    }
  }
  return defaultColors = {{"background", backgroundColor}, {"foreground", foregroundColor}};
}

ColorSettings Theme::createPackageToolBarSettingsColors(const Theme* theme) {
  return createStatusBarSettingsColors(theme);
}

ColorSettings Theme::createFindReplaceViewSettingsColors(const Theme* theme) {
   ColorSettings defaultColors  = createStatusBarSettingsColors(theme);
   defaultColors["buttonUncheckedBackgroundColor"] = getAppropriateGrey(defaultColors.value("background").name(), true);
   defaultColors["buttonCheckedBackgroundColor"] = getAppropriateGrey(defaultColors.value("background").name());

   return defaultColors;
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

  qSort(settingsWithRank.begin(), settingsWithRank.end(),
        [](std::tuple<Rank, ScopeSetting*>& s1, std::tuple<Rank, ScopeSetting*>& s2) {
          return std::get<0>(s1) > std::get<0>(s2);
        });

  QVector<ScopeSetting*> matchedSettings;
  for (std::tuple<Rank, ScopeSetting*>& setting : settingsWithRank) {
    matchedSettings.append(std::get<1>(setting));
  }

  return matchedSettings;
}

std::shared_ptr<QTextCharFormat> Theme::getFormat(const QString& scope) {
  if (scopeSettings.isEmpty())
    return nullptr;

  // check cache
  if (m_cachedFormats.contains(scope)) {
    return m_cachedFormats.value(scope);
  }

  QTextCharFormat* format = new QTextCharFormat();
  QVector<ScopeSetting*> matchedSettings = getMatchedSettings(scope);
  if (!matchedSettings.isEmpty()) {
    // foreground
    foreach (ScopeSetting* setting, matchedSettings) {
      if (setting->colorSettings->contains(foregroundStr)) {
        QColor fg = setting->colorSettings->value(foregroundStr);
        Q_ASSERT(fg.isValid());
        format->setForeground(fg);
        break;
      }
    }

    // background
    foreach (ScopeSetting* setting, matchedSettings) {
      if (setting->colorSettings->contains(backgroundStr)) {
        QColor bg = setting->colorSettings->value(backgroundStr);
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

  auto formatPtr = std::shared_ptr<QTextCharFormat>(format);
  m_cachedFormats.insert(scope, formatPtr);
  return formatPtr;
}

Rank::Rank(const QString& scopeSelector, const QString& scope) {
  if (scopeSelector.isEmpty()) {
    m_state = State::Empty;
  } else {
    QVector<QStringRef> selectors = scopeSelector.splitRef(" ");
    QVector<QStringRef> scopes = scope.splitRef(" ");
    QVector<int> scores(scopes.size(), 0);

    if (selectors.size() > scopes.size()) {
      m_state = State::Invalid;
      return;
    }

    int selectorsIndex = selectors.size() - 1, scopesIndex = scopes.size() - 1;
    bool lastMatched = false;
    for (; 0 <= selectorsIndex && 0 <= scopesIndex; scopesIndex--) {
      int rank = calcRank(selectors[selectorsIndex], scopes[scopesIndex]);
      if (rank > 0) {
        scores[scopesIndex] = rank;
        selectorsIndex--;
        lastMatched = true;
      } else if (lastMatched) {
        m_state = State::Invalid;
        return;
      }
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

bool Rank::operator>(Rank& r2) {
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

bool Rank::operator<(Rank& r2) {
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
