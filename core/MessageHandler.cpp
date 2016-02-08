#include <QMetaMethod>

#include "MessageHandler.h"

namespace core {

void MessageHandler::handler(QtMsgType type, const QMessageLogContext &, const QString &msg) {
  switch (type) {
  case QtDebugMsg:
    // this sets Qt's default message handler
    qInstallMessageHandler(0);
    qDebug() << msg;
    // Restore SilkEdit's message handler
    qInstallMessageHandler(MessageHandler::handler);
    break;
  case QtInfoMsg:
  case QtWarningMsg:
  case QtCriticalMsg:
    MessageHandler::singleton().handleMessage(type, msg);
    break;
  case QtFatalMsg:
    MessageHandler::singleton().handleMessage(type, msg);
    abort();
  }
}

void MessageHandler::handleMessage(QtMsgType type, const QString& msg) {

  static const QMetaMethod messageSignal = QMetaMethod::fromSignal(&MessageHandler::message);
  if (isSignalConnected(messageSignal)) {
    emit message(type, msg);
  } else {
    // buffer messages when no connection
    m_msgInfoList.append(MessageInfo{type, msg});
  }
}

void MessageHandler::connectNotify(const QMetaMethod& signal) {
  if (signal == QMetaMethod::fromSignal(&MessageHandler::message)) {
    for (const auto& msg : m_msgInfoList) {
      emit message(msg.type, msg.msg);
    }
    m_msgInfoList.clear();
  }
}

}  // namespace core
