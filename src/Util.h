#pragma once

#include <list>
#include <string>
#include <functional>
#include <QStringList>

#include "macros.h"

class Util {
  DISABLE_COPY_AND_MOVE(Util)

 public:
  static int binarySearch(int last, std::function<bool(int)> fn);
  static void ensureDir(const QString& path);

  /**
   * @brief Copy source to dist
   *
   * Creats parent directories of dist if they don't exist.
   *
   * @param source
   * @param dist
   * @return
   */
  static bool copy(const QString& source, const QString& dist);

  static std::list<std::string> toStdStringList(const QStringList& qStrList);

 private:
  Util() = delete;
  ~Util() = delete;
};
