#include "Console.h"
#include "ui_Console.h"
#include "Helper.h"
#include "core/MessageHandler.h"

using core::MessageHandler;

Console::Console(QWidget* parent) : QWidget(parent), ui(new Ui::Console) {
  ui->setupUi(this);
  setLayout(ui->layout);
  connect(&MessageHandler::singleton(), &MessageHandler::message,
          [=](const QString& msg) {
            ui->output->insertPlainText(msg);
            ui->output->insertPlainText("\n");
          });

  connect(ui->input, &QLineEdit::returnPressed, [=] {
    runJSCode(ui->input->text());
    ui->input->clear();
  });
}

Console::~Console() {}

void Console::runJSCode(const QString& code) {
  Helper::singleton().eval(code);
}
