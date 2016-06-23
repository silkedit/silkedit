#pragma once

#include "Helper.h"

class HelperPrivate : public QObject {
  Q_OBJECT

  char** toCStringList(const QStringList& argsStrings);

 public:
  Helper* q;

  explicit HelperPrivate(Helper* q_ptr);
  ~HelperPrivate();

  void init();

  void startNodeEventLoop();
  void emitSignal(QObject* obj, const QString& signal, QVariantList args);
  QVariant callFunc(const QString& funcName, QVariantList args = QVariantList());

  template <typename T>
  T callFunc(const QString& funcName, QVariantList args, T defaultValue);
};
