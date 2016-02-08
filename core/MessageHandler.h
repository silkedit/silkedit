#pragma once

#include <QObject>
#include <QDebug>
#include <QLoggingCategory>

#include "macros.h"
#include "Singleton.h"

#define SILKEDIT_CATEGORY "silkedit"

Q_DECLARE_LOGGING_CATEGORY(silkedit)

namespace core {

class MessageHandler : public QObject, public Singleton<MessageHandler> {
  Q_OBJECT
  DISABLE_COPY(MessageHandler)

 public:
  static void handler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

  ~MessageHandler() = default;
  DEFAULT_MOVE(MessageHandler)

  void handleMessage(const QString& msg);

 signals:
  void message(const QString& msg);

 protected:
  void connectNotify(const QMetaMethod& signal);

 private:
  friend class Singleton<MessageHandler>;
  MessageHandler() = default;

  QStringList m_storedMessages;
};

}  // namespace core
