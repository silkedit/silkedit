#pragma once

#include <functional>
#include <list>
#include <string>
#include <QStringList>
#include <QTime>
#include <QColor>
#include <QKeySequence>
#include <QVariantList>

#include "macros.h"

namespace core {

typedef QList<QByteArray> ParameterTypes;

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

  static QKeySequence toSequence(const QString& str);
  static QString toString(const QKeySequence& keySeq);
  static QString qcolorForStyleSheet(const QColor& color);

  static void processWithPublicMethods(const QMetaObject* metaObj,
                                       std::function<void(const QMetaMethod&)> fn);

  static QByteArray stripNamespace(const QByteArray& name);

  static bool matchTypes(QList<QByteArray> types, QVariantList args);

  static bool convertArgs(ParameterTypes parameterTypes, QVariantList& args);

  // Caller needs to call free argv after using it as follows
  //  free(argv[0]);
  //  free(argv);
  static char** toArgv(const QStringList& argsStrings);

  static QVariant toVariant(const char* str);
  static QVariant toVariant(const std::string &str);
  static QVariant toVariant(const QString &str);

  template <typename Func>
  static void stopWatch(Func func, const QString& msg = "time") {
    QTime startTime = QTime::currentTime();
    func();
    int passed = startTime.msecsTo(QTime::currentTime());
    qDebug("%s: %d [ms]", qPrintable(msg), passed);
  }

 private:
  friend class UtilTest;

  Util() = delete;
  ~Util() = delete;

  static bool wrappedTypeCheck(QVariant var, const QByteArray& typeName);
};

}  // namespace core
