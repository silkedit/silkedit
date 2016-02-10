#pragma once

#include <QObject>
#include <QDebug>
#include <QLoggingCategory>

#include "macros.h"
#include "Singleton.h"

#define SILKEDIT_CATEGORY "silkedit"

Q_DECLARE_LOGGING_CATEGORY(silkedit)

namespace core {

struct MessageInfo {
  QtMsgType type;
  QString msg;
};

class MessageHandler : public QObject, public Singleton<MessageHandler> {
  Q_OBJECT
  DISABLE_COPY(MessageHandler)

 public:
  static void handler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
  static void init();

  ~MessageHandler() = default;
  DEFAULT_MOVE(MessageHandler)

  void handleMessage(QtMsgType type, const QString& msg);

 signals:
  void message(QtMsgType type, const QString& msg);

 protected:
  void connectNotify(const QMetaMethod& signal);

 private:
  static QtMessageHandler s_defaultMsgHandler;

  friend class Singleton<MessageHandler>;
  MessageHandler() = default;

  QList<MessageInfo> m_storedMessages;
};

}  // namespace core
