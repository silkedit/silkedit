#pragma once

#include <memory>
#include <QMap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QTextCharFormat>

#include "macros.h"
#include "LanguageParser.h"

namespace core {

typedef QMap<QString, QColor> ColorSettings;

struct ScopeSetting {
  QString name;
  QStringList scopeSelectors;
  std::unique_ptr<ColorSettings> colorSettings;
  QFont::Weight fontWeight;
  bool isItalic;
  bool isUnderline;

  ScopeSetting()
      : colorSettings(nullptr), fontWeight(QFont::Normal), isItalic(false), isUnderline(false) {}

  bool hasFontStyle();
};

class Theme {
  DISABLE_COPY(Theme)

 public:
  Theme()
      : gutterSettings(nullptr),
        statusBarSettings(nullptr),
        tabBarSettings(nullptr),
        tabViewSettings(nullptr),
        scopeSettings(QVector<ScopeSetting*>(0)) {}
  ~Theme() = default;
  DEFAULT_MOVE(Theme)

  static Theme* loadTheme(const QString& filename);
  static int rank(const QString& scope, const QString& scope2);

  std::shared_ptr<QTextCharFormat> getFormat(const QString& scope);

  std::unique_ptr<ColorSettings> gutterSettings;
  std::unique_ptr<ColorSettings> statusBarSettings;
  std::unique_ptr<ColorSettings> tabBarSettings;
  std::unique_ptr<ColorSettings> tabViewSettings;

  QFont::Weight gutterFontWeight;
  bool isGutterItalic;
  bool isGutterUnderline;
  QString name;
  QVector<ScopeSetting*> scopeSettings;

 private:
  static ScopeSetting* matchedSetting(const QString& scope);
  QVector<ScopeSetting*> getMatchedSettings(const QString& scope);
  QMap<QString, std::shared_ptr<QTextCharFormat>> m_cachedFormats;
  static ColorSettings createGutterSettingsColors(const Theme* theme);
  static ColorSettings createStatusBarSettingsColors(const Theme* theme);
  static ColorSettings createTabBarSettingsColors(const Theme* theme);
  static ColorSettings createTabViewSettingsColors(const Theme* theme);
};

class Rank {
  enum class State { Valid, Invalid, Empty };

 public:
  Rank(const QString& scopeSelector, const QString& scope);
  ~Rank() = default;

  bool isValid() { return m_state == State::Valid || m_state == State::Empty; }
  bool isInvalid() { return m_state == State::Invalid; }
  bool isEmpty() { return m_state == State::Empty; }

  bool operator>(Rank& r);
  bool operator<(Rank& r);
  bool operator==(Rank& r);
  bool operator>=(Rank&) { throw std::runtime_error("operator >= not implemented"); }
  bool operator<=(Rank&) { throw std::runtime_error("operator <= not implemented"); }
  bool operator!=(Rank&) { throw std::runtime_error("operator != not implemented"); }

 private:
  static int calcRank(const QStringRef& scopeSelector, const QStringRef& singleScope);

  QVector<int> m_scores;
  State m_state;
};

}  // namespace core
