#include <string>
#include <QMetaMethod>

#include "MessageHandler.h"

Q_LOGGING_CATEGORY(silkedit, SILKEDIT_CATEGORY)

namespace core {

QtMessageHandler MessageHandler::s_defaultMsgHandler = nullptr;

void MessageHandler::handler(QtMsgType type,
                             const QMessageLogContext& context,
                             const QString& msg) {
  if (context.category && strcmp(context.category, SILKEDIT_CATEGORY) == 0) {
    switch (type) {
      case QtDebugMsg:
        s_defaultMsgHandler(type, context, msg);
        break;
      case QtInfoMsg:
      case QtWarningMsg:
      case QtCriticalMsg:
        MessageHandler::singleton().handleMessage(type, msg);
        s_defaultMsgHandler(type, context, msg);
        break;
      case QtFatalMsg:
        MessageHandler::singleton().handleMessage(type, msg);
        s_defaultMsgHandler(type, context, msg);
        abort();
    }
  } else {
    s_defaultMsgHandler(type, context, msg);
  }
}

void MessageHandler::init() {
  qSetMessagePattern(
      "[%{time h:mm:ss.zzz} "
      "%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-"
      "fatal}F%{endif}] %{file}:%{line} - %{message}");
  s_defaultMsgHandler = qInstallMessageHandler(MessageHandler::handler);
  Q_ASSERT(s_defaultMsgHandler);
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
