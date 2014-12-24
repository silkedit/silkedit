#include <algorithm>

#include "ThemeProvider.h"
#include "Theme.h"

std::unordered_map<QString, std::unique_ptr<Theme>> ThemeProvider::m_nameThemeMap;

QVector<QString> ThemeProvider::sortedThemeNames()
{
  QVector<QString> names(0);
  for (auto& pair : m_nameThemeMap) {
    names.push_back(pair.first);
  }

  std::sort(names.begin(), names.end());
  return names;
}

void ThemeProvider::loadTheme(const QString& fileName) {
  Theme* theme = Theme::loadTheme(fileName);
  if (theme) {
    m_nameThemeMap.insert(std::make_pair(theme->name, std::move(std::unique_ptr<Theme>(theme))));
  } else {
    qWarning("failed to load %s", qPrintable(fileName));
  }
}

Theme *ThemeProvider::theme(const QString &name)
{
  if (m_nameThemeMap.find(name) != m_nameThemeMap.end()) {
    return m_nameThemeMap.at(name).get();
  } else {
    qDebug("%s not found", qPrintable(name));
    return nullptr;
  }
}
