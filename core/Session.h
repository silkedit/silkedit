#pragma once

#include <QObject>
#include <QFont>

#include "macros.h"
#include "Singleton.h"

namespace core {

class Theme;

/**
 * @brief Session holds the temporary per user settings like current font and theme.
 */
class Session : public QObject, public Singleton<Session> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Session)

 public:
  ~Session() = default;

  // accessor
  Theme* theme() { return m_theme; }
  void setTheme(Theme* theme);

  QFont font() { return m_font; }
  void setFont(const QFont& font);

  int tabWidth() { return m_tabWidth; }
  void setTabWidth(int tabWidth);

  bool indentUsingSpaces() { return m_indentUsingSpaces; }
  void setIndentUsingSpaces(bool value);

  void init();

signals:
  void themeChanged(Theme* newTheme);
  void fontChanged(QFont font);
  void tabWidthChanged(int tabWidth);
  void indentUsingSpacesChanged(bool indentUsingSpaces);

 private:
  friend class Singleton<Session>;
  Session();

  Theme* m_theme;
  QFont m_font;
  int m_tabWidth;
  bool m_indentUsingSpaces;
};

}  // namespace core
