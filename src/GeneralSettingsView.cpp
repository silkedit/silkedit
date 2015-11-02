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
            //            qDebug("themeSelected: %s", qPrintable(text));
            Theme* theme = ThemeProvider::theme(text);
            Session::singleton().setTheme(theme);
          });
  if (Session::singleton().theme()) {
    ui->themeCombo->setCurrentText(Session::singleton().theme()->name);
  }

  // Init font combo box
  connect(ui->fontComboBox, &QFontComboBox::currentFontChanged,
          [=](const QFont& font) { Session::singleton().setFont(font); });
  qDebug() << Session::singleton().font().family();
  ui->fontComboBox->setCurrentFont(Session::singleton().font());

  // Init font size spin box
  connect(ui->fontSizeSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [=](int value) {
            QFont font = Session::singleton().font();
            font.setPointSize(value);
            Session::singleton().setFont(font);
          });
  ui->fontSizeSpin->setValue(Session::singleton().font().pointSize());
}

GeneralSettingsView::~GeneralSettingsView() {
  delete ui;
}
