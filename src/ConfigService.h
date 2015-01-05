#pragma once

#include <unordered_map>

#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

class QString;

class ConfigService {
  DISABLE_COPY_AND_MOVE(ConfigService)

 public:
  static void load();
  static bool isTrue(const QString& key);
  static QString value(const QString& key, const QString& defaultValue = "");
  static QString theme();

 private:
  ConfigService() = delete;
  ~ConfigService() = delete;

  static std::unordered_map<QString, QString> m_configs;

  static void load(const QString& filename);
};
