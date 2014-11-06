#pragma once

#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <QString>

#include "macros.h"
#include "singleton.h"
#include "stlSpecialization.h"
#include "CommandEvent.h"

class KeymapService : public Singleton<KeymapService> {
  DISABLE_COPY_AND_MOVE(KeymapService)

 public:
  ~KeymapService() = default;

  void load(const QString& filename);
  void dispatch(const QString& key);

 private:
  friend class Singleton<KeymapService>;
  KeymapService() = default;

  std::unordered_map<QString, QVariant> parseArgs(const YAML::Node& argsNode);

  std::unordered_map<QString, CommandEvent> m_keymaps;
};
