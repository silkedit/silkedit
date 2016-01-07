#pragma once

#include <node.h>
#include <v8.h>
#include <uv.h>
#include <boost/optional.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <tuple>
#include <sstream>
#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <QQueue>
#include <QMetaMethod>

#include "core/CommandArgument.h"
#include "core/macros.h"
#include "core/stlSpecialization.h"
#include "core/Singleton.h"
#include "core/Icondition.h"
#include "core/QVariantArgument.h"
#include "core/qdeclare_metatype.h"

typedef std::tuple<bool, std::string, CommandArgument> CommandEventFilterResult;

Q_DECLARE_METATYPE(CommandEventFilterResult)

class UvEventLoop : public QObject {
  Q_OBJECT

 public:
  // Don't set parent because QObject with parent can't use moveToThread
  UvEventLoop() : m_isFinished(false) {}
  ~UvEventLoop() = default;
  DEFAULT_COPY_AND_MOVE(UvEventLoop)

  void exec();

 public slots:
  void quit() { m_isFinished = true; }

 private:
  bool m_isFinished;
};

Q_DECLARE_METATYPE(std::shared_ptr<UvEventLoop>)

// This class is thread safe
class InvokeMethodInfo : public QObject {
  Q_OBJECT

 public:
  InvokeMethodInfo(QObject* anObject, const QMetaMethod& aMethod, QVariantList arguments)
      : QObject() {
    QMutexLocker locker(&m_mutex);
    m_object = anObject;
    m_method = aMethod;
    m_args = arguments;
  }

  QObject* object() {
    QMutexLocker locker(&m_mutex);
    return m_object;
  }

  const QMetaMethod& method() {
    QMutexLocker locker(&m_mutex);
    return m_method;
  }

  QVariantList args() {
    QMutexLocker locker(&m_mutex);
    return m_args;
  }

  QVariant returnValue() {
    QMutexLocker locker(&m_mutex);
    return m_returnValue;
  }

  void setReturnValue(QVariant value) {
    QMutexLocker locker(&m_mutex);
    m_returnValue = value;
  }

 private:
  QObject* m_object;
  QMetaMethod m_method;
  QVariantList m_args;
  QVariant m_returnValue;
  QMutex m_mutex;
};

Q_DECLARE_METATYPE(std::shared_ptr<InvokeMethodInfo>)

// This class is thread safe
class NODE_EXTERN NodeEmitSignal : public QObject {
  Q_OBJECT

 public:
  NodeEmitSignal(QObject* anObj, const QString& aSignal, QVariantList arguments) {
    QMutexLocker locker(&m_mutex);
    m_obj = anObj;
    m_signal = aSignal;
    m_args = arguments;
  }

  QObject* object() {
    QMutexLocker locker(&m_mutex);
    return m_obj;
  }

  QString signal() {
    QMutexLocker locker(&m_mutex);
    return m_signal;
  }

  QVariantList args() {
    QMutexLocker locker(&m_mutex);
    return m_args;
  }

 signals:
  void finished();

private:
  QObject* m_obj;
  QString m_signal;
  QVariantList m_args;
  QMutex m_mutex;
};

Q_DECLARE_METATYPE(std::shared_ptr<NodeEmitSignal>)

// This class is thread safe
class NODE_EXTERN NodeMethodCall : public QObject {
  Q_OBJECT
 public:
  NodeMethodCall() = default;

  NodeMethodCall(const QString& method, const QVariantList& args) {
    QMutexLocker locker(&m_mutex);
    m_method = method;
    m_args = args;
  }

  ~NodeMethodCall() = default;

  QString method() {
    QMutexLocker locker(&m_mutex);
    return m_method;
  }

  void setMethod(const QString& method) {
    QMutexLocker locker(&m_mutex);
    m_method = method;
  }

  QVariantList args() {
    QMutexLocker locker(&m_mutex);
    return m_args;
  }

  void setArgs(const QVariantList args) {
    QMutexLocker locker(&m_mutex);
    m_args = args;
  }

  QVariant returnValue() {
    QMutexLocker locker(&m_mutex);
    return m_returnValue;
  }
  void setReturnValue(const QVariant& value) {
    QMutexLocker locker(&m_mutex);
    m_returnValue = value;
    emit finished();
  }

 signals:
  void finished();

 private:
  QString m_method;
  QVariantList m_args;
  QVariant m_returnValue;
  QMutex m_mutex;
};
Q_DECLARE_METATYPE(std::shared_ptr<NodeMethodCall>)

class HelperPrivate;

class GetRequestResponse : public QObject {
  Q_OBJECT
  DISABLE_COPY(GetRequestResponse)

 public:
  GetRequestResponse() = default;
  ~GetRequestResponse() { qDebug("~GetRequestResponse"); }
  DEFAULT_MOVE(GetRequestResponse)

 signals:
  void onSucceeded(const QString& body);
  void onFailed(const QString& error);
};
Q_DECLARE_METATYPE(GetRequestResponse*)

class BoolResponse : public QObject {
  Q_OBJECT

 public:
  bool result() { return m_result; }

public slots:
  void setResult(bool result) {
    m_result = result;
    emit finished();
  }

 signals:
  void finished();

 private:
  bool m_result;
};
Q_DECLARE_METATYPE(BoolResponse*)

// This class is thread safe
template <typename T>
class ConcurrentQueue {
 public:
  void enqueue(const T& t) {
    QMutexLocker locker(&m_mutex);
    m_queue.enqueue(t);
  }

  T dequeue() {
    QMutexLocker locker(&m_mutex);
    return m_queue.dequeue();
  }

  bool isEmpty() {
    QMutexLocker locker(&m_mutex);
    return m_queue.isEmpty();
  }

 private:
  QQueue<T> m_queue;
  QMutex m_mutex;
};

/**
 * @brief Proxy object to interact with Node.js side
 * thread affinity: main thread
 */
class NODE_EXTERN Helper : public QObject, public core::Singleton<Helper> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Helper)

 public:
  uv_async_t async;
  QMutex asyncLock;
  ConcurrentQueue<QVariant> nodeRemoteCalls;
  ~Helper();

  void init();

  void sendFocusChangedEvent(const QString& viewType);
  void sendCommandEvent(const QString& command, const CommandArgument& cmdArgs);
  void runCommand(const QString& cmd, const CommandArgument& cmdArgs);
  bool askCondition(const QString& name, core::Operator op, const QString& value);
  QString translate(const QString& key, const QString& defaultValue);
  void loadPackage(const QString& pkgName);
  bool removePackage(const QString& pkgName);
  GetRequestResponse* sendGetRequest(const QString& url, int timeoutInMs);
  void reloadKeymaps();

 public slots:
  void invokeMethodFromJS(std::shared_ptr<InvokeMethodInfo> info,
                          std::shared_ptr<UvEventLoop> loop);

 private:
  std::unique_ptr<HelperPrivate> d;
  bool m_isStopped;

  friend class HelperPrivate;
  friend class core::Singleton<Helper>;
  Helper();

  void callNotifyJSFunc(const QString& funcName, const QVariantList& args = QVariantList());
  QVariant callRequestJSFunc(const QString& funcName, const QVariantList& args = QVariantList());
  void emitSignalInternal(const QVariantList& args);
  bool isAsyncReady();

 private slots:
  void emitSignal();
  void emitSignal(const QString& arg);
};
