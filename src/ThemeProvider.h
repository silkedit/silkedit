#pragma once

#include <unordered_map>
#include <QString>

#include "macros.h"
#include "Theme.h"
#include "stlSpecialization.h"

class ThemeProvider {
  DISABLE_COPY_AND_MOVE(ThemeProvider)

 public:
  static void loadTheme(const QString& fileName);

 private:
  static std::unordered_map<QString, std::unique_ptr<Theme>> m_nameThemeMap;

  ThemeProvider() = delete;
  ~ThemeProvider() = delete;
};
