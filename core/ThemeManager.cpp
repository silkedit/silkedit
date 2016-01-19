#include <algorithm>
#include <QDir>

#include "ThemeManager.h"
#include "Theme.h"
#include "Constants.h"

namespace core {

std::unordered_map<QString, std::unique_ptr<Theme>> ThemeManager::s_nameThemeMap;

QStringList ThemeManager::sortedThemeNames() {
  QStringList names;
  for (auto& pair : s_nameThemeMap) {
    names.push_back(pair.first);
  }

  std::sort(names.begin(), names.end());
  return names;
}

void ThemeManager::loadTheme(const QString& fileName) {
  Theme* theme = Theme::loadTheme(fileName);
  if (theme) {
    s_nameThemeMap.insert(std::make_pair(theme->name, std::move(std::unique_ptr<Theme>(theme))));
  } else {
    qWarning("failed to load %s", qPrintable(fileName));
  }
}

Theme* ThemeManager::theme(const QString& name) {
  if (s_nameThemeMap.find(name) != s_nameThemeMap.end()) {
    return s_nameThemeMap.at(name).get();
  } else {
    qDebug("%s not found", qPrintable(name));
    return nullptr;
  }
}

void ThemeManager::load() {
  for (const QString& path : Constants::singleton().themePaths()) {
    load(path);
  }
}

void core::ThemeManager::load(const QString& path) {
  QDir themesDir(path);
  if (themesDir.exists()) {
    for (const QString& themeFile : themesDir.entryList(QStringList{"*.tmTheme"})) {
      loadTheme(themesDir.filePath(themeFile));
    }
  }

  // find in sub directories
  for (const QString& subdir : themesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    qDebug("find sub directory: %s", qPrintable(subdir));
    load(themesDir.filePath(subdir));
  }
}

}  // namespace core
