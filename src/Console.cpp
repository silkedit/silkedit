#include "Console.h"
#include "ui_Console.h"
#include "core/MessageHandler.h"

using core::MessageHandler;

Console::Console(QWidget* parent) : QWidget(parent), ui(new Ui::Console) {
  ui->setupUi(this);
  setLayout(ui->layout);
  connect(&MessageHandler::singleton(), &MessageHandler::message,
          [=](QtMsgType, const QString& msg) {
            ui->output->insertPlainText(msg);
            ui->output->insertPlainText("\n");
          });
}

Console::~Console() {}
