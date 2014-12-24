#pragma once

#include <QObject>

#include "macros.h"
#include "Singleton.h"
#include "Theme.h"

class Session : public QObject, public Singleton<Session> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Session)

 public:
  ~Session() = default;

  // accessor
  Theme* theme() { return m_theme; }
  void setTheme(Theme* theme);

  void init();

signals:
  void themeChanged(Theme* newTheme);

 private:
  friend class Singleton<Session>;
  Session();

  Theme* m_theme;
};
