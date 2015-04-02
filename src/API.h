#pragma once

#include <msgpack/rpc/protocol.h>
#include <functional>
#include <unordered_map>
#include <string>

#include "macros.h"
#include "stlSpecialization.h"

class API {
  DISABLE_COPY_AND_MOVE(API)

 public:
  static void init();
  static void hideActiveFindReplacePanel();
  static void call(const QString& method, const msgpack::object& obj);
  static void call(const QString& method, msgpack::rpc::msgid_t msgId, const msgpack::object& obj);

 private:
  API() = delete;
  ~API() = delete;

  static std::unordered_map<QString, std::function<void(msgpack::object)>> s_notifyFunctions;
  static std::unordered_map<QString, std::function<void(msgpack::rpc::msgid_t, msgpack::object)>>
      s_requestFunctions;

  // notify functions
  static void alert(msgpack::object obj);
  static void loadMenu(msgpack::object obj);
  static void registerCommands(msgpack::object obj);
  static void registerContext(msgpack::object obj);
  static void unregisterContext(msgpack::object obj);
  static void open(msgpack::object obj);
  static void dispatchCommand(msgpack::object obj);

  // request functions
  static void activeView(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void activeTabView(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void activeTabViewGroup(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void activeWindow(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void showFileAndDirectoryDialog(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void windows(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void getConfig(msgpack::rpc::msgid_t msgId, msgpack::object obj);
};
