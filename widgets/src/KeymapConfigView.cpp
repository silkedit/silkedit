#include <QTableWidgetItem>
#include <QFile>

#include "KeymapConfigView.h"
#include "ui_KeymapConfigView.h"
#include "KeymapManager.h"
#include "Helper.h"
#include "DocumentManager.h"
#include "Window.h"
#include "core/Constants.h"

using core::Constants;

KeymapConfigView::KeymapConfigView(QWidget* parent)
    : CustomWidget(parent), ui(new Ui::KeymapConfigView) {
  ui->setupUi(this);

  connect(ui->filterLine, &QLineEdit::textEdited, ui->keymapTable, &KeymapTableView::setFilterText);
  connect(ui->openKeymapFileButton, &QPushButton::clicked, this, [=] {
    QFile keymapFile(Constants::singleton().userKeymapPath());
    if (!keymapFile.exists()) {
      if (keymapFile.open(QFile::WriteOnly | QIODevice::Text)) {
        QString defaultContent(R"(
# Define your custom keymap
# - { key: ctrl+b, command: move_cursor_left }
)");

        QTextStream out(&keymapFile);
        out << defaultContent.trimmed();
      } else {
        qWarning("Failed to open %s", qPrintable(keymapFile.fileName()));
      }
    }
    Window::showFirst();
    DocumentManager::singleton().open(Constants::singleton().userKeymapPath());
  });
}

KeymapConfigView::~KeymapConfigView() {
  delete ui;
}
