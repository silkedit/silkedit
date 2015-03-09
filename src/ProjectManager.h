#pragma once

#include <QString>

#include "macros.h"

class ProjectManager {
  DISABLE_COPY_AND_MOVE(ProjectManager)

 public:
  static bool open(const QString& directoryName);

 private:
  ProjectManager() = delete;
  ~ProjectManager() = delete;
};
