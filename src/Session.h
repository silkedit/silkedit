#pragma once

#include <QObject>

#include "macros.h"
#include "Singleton.h"
#include "Theme.h"

/**
 * @brief The Session class
 * Session holds the temporary per user settings like current font and theme.
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

  void init();

signals:
  void themeChanged(Theme* newTheme);
  void fontChanged(QFont font);

 private:
  friend class Singleton<Session>;
  Session();

  Theme* m_theme;
  QFont m_font;
};
