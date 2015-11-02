#include "GeneralSettingsView.h"
#include "ui_GeneralSettingsView.h"
#include "core/ThemeProvider.h"
#include "core/Session.h"

using core::ThemeProvider;
using core::Theme;
using core::Session;

GeneralSettingsView::GeneralSettingsView(QWidget* parent)
    : QWidget(parent), ui(new Ui::GeneralSettingsView) {
  ui->setupUi(this);

  // Init theme combo box
  ui->themeCombo->addItems(ThemeProvider::sortedThemeNames());
  connect(ui->themeCombo,
          static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
          [=](const QString& text) {
            qDebug("themeSelected: %s", qPrintable(text));
            Theme* theme = ThemeProvider::theme(text);
            Session::singleton().setTheme(theme);
          });

  // Select current theme
  if (Session::singleton().theme()) {
    ui->themeCombo->setCurrentText(Session::singleton().theme()->name);
  }
}

GeneralSettingsView::~GeneralSettingsView() {
  delete ui;
}
