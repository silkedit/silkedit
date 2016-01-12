#pragma once

#include <string>
#include <tuple>
#include <QObject>

#include "core/CommandArgument.h"
#include "core/macros.h"
#include "core/stlSpecialization.h"
#include "core/Singleton.h"
#include "core/Icondition.h"
#include "core/QVariantArgument.h"
#include "core/qdeclare_metatype.h"

namespace atom {
class NodeBindings;
}

typedef std::tuple<bool, std::string, CommandArgument> CommandEventFilterResult;

Q_DECLARE_METATYPE(CommandEventFilterResult)

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

/**
 * @brief Proxy object to interact with Node.js side
 */
class Helper : public QObject, public core::Singleton<Helper> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(Helper)

 public:
  ~Helper();

  void init();
  void cleanup();
  void sendFocusChangedEvent(const QString& viewType);
  void sendCommandEvent(const QString& command, const CommandArgument& cmdArgs);
  void runCommand(const QString& cmd, const CommandArgument& cmdArgs);
  bool askCondition(const QString& name, core::Operator op, const QString& value);
  QString translate(const QString& key, const QString& defaultValue);
  void loadPackage(const QString& pkgName);
  bool removePackage(const QString& pkgName);
  GetRequestResponse* sendGetRequest(const QString& url, int timeoutInMs);
  void reloadKeymaps();
  void quitApplication();

public slots:
  void uvRunOnce();

 private:
  std::unique_ptr<HelperPrivate> d;
  atom::NodeBindings* m_nodeBindings;

  friend class HelperPrivate;
  friend class core::Singleton<Helper>;
  Helper();

  void emitSignalInternal(const QVariantList& args);

 private slots:
  void emitSignal();
  void emitSignal(const QString& arg);
};
