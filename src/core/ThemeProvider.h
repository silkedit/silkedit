#pragma once

#include <unordered_map>
#include <QString>

#include "core/macros.h"
#include "core/Theme.h"
#include "core/stlSpecialization.h"

namespace core {

class ThemeProvider {
  DISABLE_COPY_AND_MOVE(ThemeProvider)

 public:
  // accessor
  static QVector<QString> sortedThemeNames();
  static void loadTheme(const QString& fileName);
  static Theme* theme(const QString& name);

 private:
  static std::unordered_map<QString, std::unique_ptr<Theme>> s_nameThemeMap;

  ThemeProvider() = delete;
  ~ThemeProvider() = delete;
};

}  // namespace core
