#pragma once

#include <QString>

#include "macros.h"

class PackageManager {
  DISABLE_COPY_AND_MOVE(PackageManager)

 public:
  static void loadPackages();

 private:
  PackageManager() = delete;
  ~PackageManager() = delete;

  static void loadPackages(const QString& dirName);
};
