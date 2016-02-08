#pragma once

#include <QObject>
#include <QDebug>

#include "macros.h"
#include "Singleton.h"

namespace core {

struct MessageInfo {
  QtMsgType type;
  QString msg;
};

class MessageHandler : public QObject, public Singleton<MessageHandler> {
  Q_OBJECT
  DISABLE_COPY(MessageHandler)

 public:
  static void handler(QtMsgType type, const QMessageLogContext&, const QString& msg);

  ~MessageHandler() = default;
  DEFAULT_MOVE(MessageHandler)

  void handleMessage(QtMsgType type, const QString& msg);

 signals:
  void message(QtMsgType type, const QString& msg);

 protected:
  void connectNotify(const QMetaMethod& signal);

 private:
  friend class Singleton<MessageHandler>;
  MessageHandler() = default;

  QList<MessageInfo> m_msgInfoList;
};

}  // namespace core
