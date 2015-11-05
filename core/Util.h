﻿#pragma once

#include <list>
#include <string>
#include <functional>
#include <QStringList>
#include <QTime>

#include "macros.h"

namespace core {

class Util {
  DISABLE_COPY_AND_MOVE(Util)

 public:
  static size_t binarySearch(size_t last, std::function<bool(size_t)> fn);
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

  template <typename Func>
  static void stopWatch(Func func, const QString& msg = "time") {
    QTime startTime = QTime::currentTime();
    func();
    int passed = startTime.msecsTo(QTime::currentTime());
    qDebug("%s: %d [ms]", qPrintable(msg), passed);
  }

 private:
  Util() = delete;
  ~Util() = delete;
};

}  // namespace core