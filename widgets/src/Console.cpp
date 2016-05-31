#include <QCompleter>
#include <QVBoxLayout>

#include "Console.h"
#include "ui_Console.h"
#include "Helper.h"
#include "core/MessageHandler.h"
#include "core/Theme.h"
#include "core/Config.h"
#include "core/Util.h"

using core::MessageHandler;
using core::Theme;
using core::Config;
using core::Util;

Console::Console(QWidget* parent) : CustomWidget(parent), ui(new Ui::Console) {
  ui->setupUi(this);
  ui->layout->setContentsMargins(0, 0, 0, 0);
  ui->layout->setSpacing(0);
  ui->layout->setMargin(0);
  setLayout(ui->layout);

  QCompleter* completer = new QCompleter(this);
  completer->setModel(&m_historyModel);
  ui->input->setCompleter(completer);

  connect(&MessageHandler::singleton(), &MessageHandler::message, this,
          [=](QtMsgType type, QString msg) {
            const QString& htmlMsg = msg.replace(QStringLiteral("\n"), QStringLiteral("<br>"));
            QString text = QStringLiteral("<div style='color:%1;'>%2</div>");
            QString color;
            switch (type) {
              case QtDebugMsg:
              case QtInfoMsg:
                color = QStringLiteral("black");
                break;
              case QtWarningMsg:
                color = QStringLiteral("#9F6000");
                break;
              case QtCriticalMsg:
              case QtFatalMsg:
                color = QStringLiteral("red");
                break;
            }
            ui->output->append(text.arg(color).arg(htmlMsg));
          });

  connect(ui->input, &QLineEdit::returnPressed, this, [=] {
    runJSCode(ui->input->text());
    m_historyModel.prepend(ui->input->text());
    ui->input->clear();
  });

  connect(&Config::singleton(), &Config::themeChanged, this, &Console::setTheme);

  setTheme(Config::singleton().theme());
}

Console::~Console() {}

void Console::showEvent(QShowEvent*) {
  ui->input->setFocus();
}

void Console::runJSCode(const QString& code) {
  Helper::singleton().eval(code);
}

void Console::setTheme(core::Theme* theme) {
  if (theme->consoleSettings != nullptr) {
    const auto& style = QStringLiteral(R"(
#%1 {
  background-color: %2;
}

#%1 #output {
  margin: 2px 2px 2px 2px;
  border: 1px solid transparent;
  border-radius: 2px;
}

#%1 #input {
  margin: 0 2px 2px 2px;
  padding: 0 0 0 2px;
  border: 1px solid transparent;
  border-radius: 2px;
}
)")
                            .arg(this->objectName())
                            .arg(Util::qcolorForStyleSheet(
                                theme->consoleSettings->value(QStringLiteral("background"))));
    setStyleSheet(style);
  }
}
