#include "ThemeProvider.h"
#include "Theme.h"

std::unordered_map<QString, std::unique_ptr<Theme>> ThemeProvider::m_nameThemeMap;

void ThemeProvider::loadTheme(const QString& fileName) {
  Theme* theme = Theme::loadTheme(fileName);
  if (theme) {
    m_nameThemeMap.insert(std::make_pair(theme->name, std::move(std::unique_ptr<Theme>(theme))));
  } else {
    qWarning("failed to load %s", qPrintable(fileName));
  }
}
