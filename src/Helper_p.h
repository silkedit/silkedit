#pragma once

#include <QThread>

#include "Helper.h"
#include "core/IKeyEventFilter.h"

class HelperThread : public QThread {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(HelperThread)

 public:
  explicit HelperThread(QObject* parent);
  ~HelperThread() = default;

 protected:
  void run() override;
};

class HelperPrivate : public QObject, public core::IKeyEventFilter {
  Q_OBJECT

 public:
  Helper* q;
  QThread* m_helperThread;

  explicit HelperPrivate(Helper* q_ptr);
  ~HelperPrivate();

  void init();

  // IKeyEventFilter interface
  bool keyEventFilter(QKeyEvent* event) override;
  void onFinished();
  CommandEventFilterResult cmdEventFilter(const std::string& name, const CommandArgument& arg);
  void startHelperThread();
  void cacheMethods(const QString& className, const QMetaObject* object);
};
