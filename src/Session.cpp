#include "Session.h"
#include "ThemeProvider.h"
#include "core/ConfigManager.h"

using core::ConfigManager;

void Session::setTheme(Theme* theme) {
  if (m_theme != theme) {
    m_theme = theme;
    emit themeChanged(theme);
  }
}

void Session::setFont(const QFont& font) {
  if (m_font != font) {
    m_font = font;
    emit fontChanged(font);
  }
}

void Session::setTabWidth(int tabWidth) {
  if (m_tabWidth != tabWidth) {
    m_tabWidth = tabWidth;
    emit tabWidthChanged(tabWidth);
  }
}

void Session::setIndentUsingSpaces(bool value) {
  if (m_indentUsingSpaces != value) {
    m_indentUsingSpaces = value;
    emit indentUsingSpacesChanged(value);
  }
}

void Session::init() {
  setTheme(ThemeProvider::theme(ConfigManager::theme()));
  QFont font(ConfigManager::fontFamily(), ConfigManager::fontSize());
  setFont(font);
  setTabWidth(ConfigManager::tabWidth());
  setIndentUsingSpaces(ConfigManager::indentUsingSpaces());
}

Session::Session() : m_theme(nullptr) {
}
