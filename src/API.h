#pragma once

#include <msgpack/rpc/protocol.h>
#include <functional>
#include <unordered_map>
#include <string>

#include "macros.h"

class API {
  DISABLE_COPY_AND_MOVE(API)

 public:
  static void init();
  static void hideActiveFindReplacePanel();
  static void call(const std::string& method, const msgpack::object& obj);
  static void call(const std::string& method,
                   msgpack::rpc::msgid_t msgId,
                   const msgpack::object& obj);

 private:
  API() = delete;
  ~API() = delete;

  static std::unordered_map<std::string, std::function<void(msgpack::object)>> notifyFunctions;
  static std::unordered_map<std::string,
                            std::function<void(msgpack::rpc::msgid_t, msgpack::object)>>
      requestFunctions;

  // notify functions
  static void alert(msgpack::object obj);
  static void loadMenu(msgpack::object obj);
  static void registerCommands(msgpack::object obj);
  static void open(msgpack::object obj);

  // request functions
  static void activeView(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void activeTabView(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void activeWindow(msgpack::rpc::msgid_t msgId, msgpack::object obj);
  static void showFileAndDirectoryDialog(msgpack::rpc::msgid_t msgId, msgpack::object obj);
};
