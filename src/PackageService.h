#pragma once

#include <QString>

#include "macros.h"

class PackageService {
  DISABLE_COPY_AND_MOVE(PackageService)

 public:
  static void loadPackages(const QString& dirName = "packages");

 private:
  PackageService() = delete;
  ~PackageService() = delete;
};
