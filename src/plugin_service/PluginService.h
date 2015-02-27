#pragma once

#include <msgpack/rpc/protocol.h>
#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include <QLocalServer>

#include "macros.h"
#include "Singleton.h"

class PluginService : public QObject, public Singleton<PluginService> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(PluginService)

 public:
  ~PluginService();

  void init();
  void callExternalCommand(const QString& cmd);

  template <typename Result, typename Error>
  void sendResponse(const Result& res, const Error& err, msgpack::rpc::msgid_t id) {
    msgpack::sbuffer sbuf;
    msgpack::rpc::msg_response<Result, Error> response;
    response.msgid = id;
    response.error = err;
    response.result = res;

    msgpack::pack(sbuf, response);

    m_socket->write(sbuf.data(), sbuf.size());
  }

 private:
  friend class Singleton<PluginService>;
  PluginService();

  QProcess* m_pluginProcess;
  QLocalSocket* m_socket;
  QLocalServer* m_server;

 private:
 private slots:
  void readStdout();
  void readStderr();
  void pluginRunnerConnected();
  void error(QProcess::ProcessError error);
  void readRequest();
  void displayError(QLocalSocket::LocalSocketError);
};
