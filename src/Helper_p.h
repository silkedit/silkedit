#pragma once

#include "Helper.h"
#include "core/IKeyEventFilter.h"

namespace node {
class ArrayBufferAllocator;
}

class HelperPrivate : public QObject, public core::IKeyEventFilter {
  Q_OBJECT

 public:
  Helper* q;

  explicit HelperPrivate(Helper* q_ptr);
  ~HelperPrivate();

  void init();

  // IKeyEventFilter interface
  bool keyEventFilter(QKeyEvent* event) override;
  CommandEventFilterResult cmdEventFilter(const std::string& name, const CommandArgument& arg);
  void startNodeEventLoop();
  void startNodeInstance(void* arg);
  void quitApplication();
  void cacheMethods(const QString& className, const QMetaObject* object);
  void cleanup();
};
