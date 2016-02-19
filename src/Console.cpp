#include <QCompleter>
#include <QVBoxLayout>

#include "Console.h"
#include "ui_Console.h"
#include "Helper.h"
#include "core/MessageHandler.h"

using core::MessageHandler;

Console::Console(QWidget* parent) : QWidget(parent), ui(new Ui::Console) {
  ui->setupUi(this);
  ui->layout->setContentsMargins(0, 0, 0, 0);
  ui->layout->setSpacing(0);
  ui->layout->setMargin(0);
  setLayout(ui->layout);

  QCompleter* completer = new QCompleter(this);
  completer->setModel(&m_historyModel);
  ui->input->setCompleter(completer);

  connect(&MessageHandler::singleton(), &MessageHandler::message, this,
          [=](QtMsgType type, const QString& msg) {
            QString text = "<div style='color:%1;'>%2</div>";
            QString color;
            switch (type) {
              case QtDebugMsg:
              case QtInfoMsg:
                color = "black";
                break;
              case QtWarningMsg:
                color = "yellow";
                break;
              case QtCriticalMsg:
              case QtFatalMsg:
                color = "red";
                break;
            }
            ui->output->append(text.arg(color).arg(msg));
          });

  connect(ui->input, &QLineEdit::returnPressed, this, [=] {
    runJSCode(ui->input->text());
    m_historyModel.prepend(ui->input->text());
    ui->input->clear();
  });
}

Console::~Console() {}

void Console::showEvent(QShowEvent *)
{
  ui->input->setFocus();
}

void Console::runJSCode(const QString& code) {
  Helper::singleton().eval(code);
}
