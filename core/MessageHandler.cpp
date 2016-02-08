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
        qDebug() << msg;
        // Restore SilkEdit's message handler
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtInfoMsg:
      case QtWarningMsg:
      case QtCriticalMsg:
        MessageHandler::singleton().handleMessage(msg);
        break;
      case QtFatalMsg:
        MessageHandler::singleton().handleMessage(msg);
        abort();
    }
  } else {
    switch (type) {
      case QtDebugMsg:
        // this sets Qt's default message handler
        qInstallMessageHandler(0);
        qDebug() << msg;
        // Restore SilkEdit's message handler
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtInfoMsg:
        qInstallMessageHandler(0);
        qInfo() << msg;
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtWarningMsg:
        qInstallMessageHandler(0);
        qWarning() << msg;
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtCriticalMsg:
        qInstallMessageHandler(0);
        qCritical() << msg;
        qInstallMessageHandler(MessageHandler::handler);
        break;
      case QtFatalMsg:
        qInstallMessageHandler(0);
        qFatal("%s", msg.toUtf8().constData());
        qInstallMessageHandler(MessageHandler::handler);
        abort();
    }
  }
}

void MessageHandler::handleMessage(const QString& msg) {
  static const QMetaMethod messageSignal = QMetaMethod::fromSignal(&MessageHandler::message);
  if (isSignalConnected(messageSignal)) {
    emit message(msg);
  } else {
    // buffer messages when no connection
    m_storedMessages.append(msg);
  }
}

void MessageHandler::connectNotify(const QMetaMethod& signal) {
  if (signal == QMetaMethod::fromSignal(&MessageHandler::message)) {
    for (const auto& msg : m_storedMessages) {
      emit message(msg);
    }
    m_storedMessages.clear();
  }
}

}  // namespace core
