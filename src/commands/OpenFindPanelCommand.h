#pragma once

#include <QObject>

#include "ICommand.h"

class OpenFindPanelCommand : public QObject, public ICommand {
  Q_OBJECT
 public:
  static constexpr auto name = "open_find_panel";

  OpenFindPanelCommand() : ICommand(name) {}
  ~OpenFindPanelCommand() = default;
  DEFAULT_COPY_AND_MOVE(OpenFindPanelCommand)

 private:
  void doRun(const CommandArgument& args, int repeat = 1) override;
};
