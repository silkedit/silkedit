#pragma once

#include <unordered_map>

#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"

class QString;

class ConfigService : public Singleton<ConfigService> {
  DISABLE_COPY_AND_MOVE(ConfigService)

 public:
  ~ConfigService() = default;

  void load(const QString& filename = "config.yml");
  bool isTrue(const QString& key);

 private:
  friend class Singleton<ConfigService>;
  ConfigService() = default;

  std::unordered_map<QString, QString> m_configs;
};
