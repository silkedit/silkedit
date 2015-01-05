#pragma once

#include <QString>

#include "macros.h"

class PackageService {
  DISABLE_COPY_AND_MOVE(PackageService)

 public:
  static void loadPackages();

 private:
  PackageService() = delete;
  ~PackageService() = delete;

  static void loadPackages(const QString& dirName);
};
