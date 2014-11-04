#pragma once

#include <unordered_map>
#include <QString>

#include "macros.h"
#include "singleton.h"
#include "stlSpecialization.h"

class KeymapService : public Singleton<KeymapService> {
  DISABLE_COPY_AND_MOVE(KeymapService)

public:
  ~KeymapService() = default;

  void load(const QString &filename);
  void dispatch(const QString &key);

private:
  friend class Singleton<KeymapService>;
  KeymapService() = default;

  std::unordered_map<QString, QString> m_keymaps;
};
