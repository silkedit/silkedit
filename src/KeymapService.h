#pragma once

#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <QString>
#include <QKeyEvent>
#include <QKeySequence>

#include "macros.h"
#include "Singleton.h"
#include "stlSpecialization.h"
#include "CommandEvent.h"
#include "ViEngine.h"

class KeymapService : public Singleton<KeymapService> {
  DISABLE_COPY_AND_MOVE(KeymapService)

 public:
  ~KeymapService() = default;

  void load(const QString& filename, ViEngine* viEngine);
  bool dispatch(const QKeyEvent& ev);

 private:
  friend class Singleton<KeymapService>;
  KeymapService() = default;

  std::unordered_map<QString, QVariant> parseArgs(const YAML::Node& argsNode);

  std::unordered_map<QKeySequence, CommandEvent> m_keymaps;
};
