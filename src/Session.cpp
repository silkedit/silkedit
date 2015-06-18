#include "Session.h"
#include "ThemeProvider.h"
#include "ConfigManager.h"

void Session::setTheme(Theme* theme) {
  m_theme = theme;
  emit themeChanged(theme);
}

void Session::setFont(const QFont& font) {
  m_font = font;
  emit fontChanged(font);
}

void Session::init() {
  setTheme(ThemeProvider::theme(ConfigManager::theme()));
  QFont font(ConfigManager::fontFamily(), ConfigManager::fontSize());
  setFont(font);
}

Session::Session() : m_theme(nullptr) {
}
