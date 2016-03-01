#pragma once

#include <memory>
#include <unordered_map>
#include <QMap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QTextCharFormat>

#include "macros.h"
#include "LanguageParser.h"
#include "stlSpecialization.h"

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
      : textEditViewSettings(nullptr),
        gutterSettings(nullptr),
        statusBarSettings(nullptr),
        tabBarSettings(nullptr),
        tabViewSettings(nullptr),
        projectTreeViewSettings(nullptr),
        windowSettings(nullptr),
        packageToolBarSettings(nullptr),
        scopeSettings(QVector<ScopeSetting*>(0)) {}
  ~Theme() = default;
  DEFAULT_MOVE(Theme)

  static Theme* loadTheme(const QString& filename);
  static int rank(const QString& scope, const QString& scope2);

  QTextCharFormat *getFormat(const QString& scope);

  std::unique_ptr<ColorSettings> textEditViewSettings;
  std::unique_ptr<ColorSettings> gutterSettings;
  std::unique_ptr<ColorSettings> statusBarSettings;
  std::unique_ptr<ColorSettings> tabBarSettings;
  std::unique_ptr<ColorSettings> tabViewSettings;
  std::unique_ptr<ColorSettings> projectTreeViewSettings;
  std::unique_ptr<ColorSettings> windowSettings;
  std::unique_ptr<ColorSettings> packageToolBarSettings;
  std::unique_ptr<ColorSettings> findReplaceViewSettings;
  std::unique_ptr<ColorSettings> consoleSettings;

  QFont::Weight gutterFontWeight;
  bool isGutterItalic;
  bool isGutterUnderline;
  QString name;
  QVector<ScopeSetting*> scopeSettings;

  bool isDarkTheme() const;

  boost::optional<QFont> font();
  void setFont(const QFont &font);

private:
  static ScopeSetting* matchedSetting(const QString& scope);

  static ColorSettings createTextEditViewSettingsColors(const Theme* theme);
  static ColorSettings createGutterSettingsColors(const Theme* theme);
  static ColorSettings createStatusBarSettingsColors(const Theme* theme);
  static ColorSettings createTabBarSettingsColors(const Theme* theme);
  static ColorSettings createTabViewSettingsColors(const Theme* theme);
  static ColorSettings createProjectTreeViewSettingsColors(const Theme* theme);
  static ColorSettings createWindowSettingsColors(const Theme* theme);
  static ColorSettings createPackageToolBarSettingsColors(const Theme* theme);
  static ColorSettings createFindReplaceViewSettingsColors(const Theme* theme);
  static ColorSettings createConsleSettingsColors(const Theme* theme);

  std::unordered_map<QString, std::unique_ptr<QTextCharFormat>> m_cachedFormats;

  // tmTheme file doesn't have a font setting.
  // Ideally, SyntaxHighlighter should have a font setting, but calling setFont in highlightBlock
  // method is VERY SLOW.
  // As a workaround, Theme keeps a font setting and apply it when creating QTextCharFormat.
  boost::optional<QFont> m_font;

  QVector<ScopeSetting*> getMatchedSettings(const QString& scope);
};

class Rank {
  enum class State { Valid, Invalid, Empty };

 public:
  Rank(const QString& scopeSelector, const QString& scope);
  ~Rank() = default;

  bool isValid() const { return m_state == State::Valid || m_state == State::Empty; }
  bool isInvalid() const { return m_state == State::Invalid; }
  bool isEmpty() const { return m_state == State::Empty; }

  bool operator>(const Rank& r) const;
  bool operator<(const Rank& r) const;
  bool operator>=(const Rank&) { throw std::runtime_error("operator >= not implemented"); }
  bool operator<=(const Rank&) { throw std::runtime_error("operator <= not implemented"); }
  bool operator!=(const Rank&) { throw std::runtime_error("operator != not implemented"); }

 private:
  static int calcRank(const QStringRef& scopeSelector, const QStringRef& singleScope);

  QVector<int> m_scores;
  State m_state;
};

}  // namespace core
