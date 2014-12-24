#include "Session.h"
#include "ThemeProvider.h"
#include "ConfigService.h"

void Session::setTheme(Theme* theme) {
  m_theme = theme;
  emit themeChanged(theme);
}

void Session::init() {
  setTheme(ThemeProvider::theme(ConfigService::theme()));
}

Session::Session() : m_theme(nullptr) {
}
