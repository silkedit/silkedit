#pragma once

#include <msgpack/rpc/protocol.h>
#include <QObject>
#include <QProcess>
#include <QLocalSocket>
#include <QLocalServer>

#include "macros.h"
#include "Singleton.h"

class PluginManager : public QObject, public Singleton<PluginManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(PluginManager)

 public:
  ~PluginManager();

  void init();

  void callExternalCommand(const QString& cmd, std::unordered_map<std::string, std::string> args);

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
  static std::unordered_map<std::string, std::function<void(const std::string&, msgpack::object)>>
      s_notifyFunctions;
  static std::unordered_map<
      std::string,
      std::function<void(msgpack::rpc::msgid_t, const std::string, msgpack::object)>>
      s_requestFunctions;

  friend class Singleton<PluginManager>;
  PluginManager();

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
