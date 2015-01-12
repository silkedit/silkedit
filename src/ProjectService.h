#pragma once

#include <QString>

#include "macros.h"

class ProjectService {
  DISABLE_COPY_AND_MOVE(ProjectService)

 public:
  static bool open(const QString& directoryName);

 private:
  ProjectService() = delete;
  ~ProjectService() = delete;
};
