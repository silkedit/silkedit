#pragma once

#include <unordered_map>
#include <QString>

#include "macros.h"
#include "Theme.h"
#include "stlSpecialization.h"

namespace core {

class ThemeManager {
  DISABLE_COPY_AND_MOVE(ThemeManager)

 public:
  // accessor
  static QStringList sortedThemeNames();
  static void loadTheme(const QString& fileName);
  static Theme* theme(const QString& name);
  static void load();

 private:
  static std::unordered_map<QString, std::unique_ptr<Theme>> s_nameThemeMap;
  static void load(const QString& path);

  ThemeManager() = delete;
  ~ThemeManager() = delete;
};

}  // namespace core
