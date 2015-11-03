#include "Session.h"
#include "ThemeProvider.h"
#include "ConfigModel.h"

namespace core {

void Session::setTheme(Theme* theme) {
  if (m_theme != theme) {
    m_theme = theme;
    ConfigModel::saveThemeName(theme->name);
    emit themeChanged(theme);
  }
}

void Session::setFont(const QFont& font) {
  if (m_font != font) {
    m_font = font;
    // http://doc.qt.io/qt-5.5/qfont.html#HintingPreference-enum
    // On Windows, FullHinting makes some fonts ugly
    // On Mac, hintingPreference is ignored.
    m_font.setHintingPreference(QFont::PreferVerticalHinting);
    ConfigModel::saveFontFamily(font.family());
    ConfigModel::saveFontSize(font.pointSize());
    emit fontChanged(m_font);
  }
}

void Session::setTabWidth(int tabWidth) {
  if (m_tabWidth != tabWidth) {
    m_tabWidth = tabWidth;
    ConfigModel::saveTabWidth(tabWidth);
    emit tabWidthChanged(tabWidth);
  }
}

void Session::setIndentUsingSpaces(bool value) {
  if (m_indentUsingSpaces != value) {
    m_indentUsingSpaces = value;
    ConfigModel::saveIndentUsingSpaces(value);
    emit indentUsingSpacesChanged(value);
  }
}

void Session::init() {
  setTheme(ThemeProvider::theme(ConfigModel::themeName()));
  QFont font(ConfigModel::fontFamily(), ConfigModel::fontSize());
  setFont(font);
  setTabWidth(ConfigModel::tabWidth());
  setIndentUsingSpaces(ConfigModel::indentUsingSpaces());
}

Session::Session() : m_theme(nullptr) {}

}  // namespace core
