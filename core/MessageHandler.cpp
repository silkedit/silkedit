#include <QMetaMethod>

#include "MessageHandler.h"

Q_LOGGING_CATEGORY(silkedit, SILKEDIT_CATEGORY)

namespace core {

void MessageHandler::handler(QtMsgType type,
                             const QMessageLogContext& context,
                             const QString& msg) {
  if (context.category == SILKEDIT_CATEGORY) {
    switch (type) {
      case QtDebugMsg:
        // this sets Qt's default message handler
        qInstallMessageHandler(0);
        QMessageLogger(context.file, context.line, context.function).debug() << msg;
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
  } else {
    switch (type) {
      case QtDebugMsg:
        // this sets Qt's default message handler
        qInstallMessageHandler(0);
        QMessageLogger(context.file, context.line, context.function).debug() << msg;
        // Restore SilkEdit's message handler
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtInfoMsg:
        qInstallMessageHandler(0);
        QMessageLogger(context.file, context.line, context.function).info() << msg;
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtWarningMsg:
        qInstallMessageHandler(0);
        QMessageLogger(context.file, context.line, context.function).warning() << msg;
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtCriticalMsg:
        qInstallMessageHandler(0);
        QMessageLogger(context.file, context.line, context.function).critical() << msg;
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtFatalMsg:
        qInstallMessageHandler(0);
        QMessageLogger(context.file, context.line, context.function).fatal("%s", msg.toUtf8().constData());
        qInstallMessageHandler(MessageHandler::handler);
        abort();
    }
  }
}

void MessageHandler::handleMessage(QtMsgType type, const QString& msg) {
  static const QMetaMethod messageSignal = QMetaMethod::fromSignal(&MessageHandler::message);
  if (isSignalConnected(messageSignal)) {
    emit message(type, msg);
  } else {
    // buffer messages when no connection
    m_storedMessages.append(MessageInfo{type, msg});
  }
}

void MessageHandler::connectNotify(const QMetaMethod& signal) {
  if (signal == QMetaMethod::fromSignal(&MessageHandler::message)) {
    for (const auto& msgInfo : m_storedMessages) {
      emit message(msgInfo.type, msgInfo.msg);
    }
    m_storedMessages.clear();
  }
}

}  // namespace core
