#include "core/Theme.h"
#include "core/PListParser.h"

namespace {
using core::ColorSettings;
using core::ScopeSetting;

const QString nameStr = "name";
const QString scopeStr = "scope";
const QString settingsStr = "settings";
const QString foregroundStr = "foreground";
const QString backgroundStr = "background";

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

namespace core {

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

  // gutterSettings
  const QString gutterSettingsStr = "gutterSettings";
  if (rootMap.contains(gutterSettingsStr)) {
    theme->gutterSettings.reset(new ColorSettings());
    parseSettings(theme->gutterSettings.get(), &(theme->gutterFontWeight), &(theme->isGutterItalic),
                  &(theme->isGutterUnderline), rootMap.value(gutterSettingsStr));
  }

  // name
  if (rootMap.contains(nameStr)) {
    theme->name = rootMap.value(nameStr).toString();
  }

  // settings
  if (rootMap.contains(settingsStr)) {
    QVariantList settingList = rootMap.value(settingsStr).toList();
    foreach (const QVariant& var, settingList) { theme->scopeSettings.append(toScopeSetting(var)); }
  }

  return theme;
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
