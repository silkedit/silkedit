#include "GeneralSettingsView.h"
#include "ui_GeneralSettingsView.h"
#include "SilkApp.h"
#include "core/ThemeProvider.h"
#include "core/Session.h"
#include "core/ConfigModel.h"

using core::ThemeProvider;
using core::Theme;
using core::Session;
using core::ConfigModel;

GeneralSettingsView::GeneralSettingsView(QWidget* parent)
    : QWidget(parent), ui(new Ui::GeneralSettingsView) {
  ui->setupUi(this);
  ui->restartButton->setVisible(false);

  // Theme combo box
  ui->themeCombo->addItems(ThemeProvider::sortedThemeNames());
  if (Session::singleton().theme()) {
    ui->themeCombo->setCurrentText(Session::singleton().theme()->name);
  }
  connect(ui->themeCombo,
          static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
          [=](const QString& text) {
            //            qDebug("themeSelected: %s", qPrintable(text));
            Theme* theme = ThemeProvider::theme(text);
            Session::singleton().setTheme(theme);
          });

  // Font combo box
  ui->fontComboBox->setCurrentFont(Session::singleton().font());
  connect(ui->fontComboBox, &QFontComboBox::currentFontChanged,
          [=](const QFont& font) { Session::singleton().setFont(font); });

  // Font size spin box
  ui->fontSizeSpin->setValue(Session::singleton().font().pointSize());
  connect(ui->fontSizeSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [=](int value) {
            QFont font = Session::singleton().font();
            font.setPointSize(value);
            Session::singleton().setFont(font);
          });

  // Indent using spaces checkbox
  ui->indentUsingSpacesCheck->setChecked(Session::singleton().indentUsingSpaces());
  connect(ui->indentUsingSpacesCheck, &QCheckBox::toggled,
          [=](bool checked) { Session::singleton().setIndentUsingSpaces(checked); });

  // Tab width spin box
  ui->tabWidthSpin->setValue(Session::singleton().tabWidth());
  connect(ui->tabWidthSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [=](int value) { Session::singleton().setTabWidth(value); });

  // Language combo box
  ui->langCombo->addItem(tr("System Language"), "system");
  ui->langCombo->addItem(tr("English"), "en");
  ui->langCombo->addItem(tr("Japanese"), "ja");
  int index = ui->langCombo->findData(ConfigModel::locale());
  if (index == -1) {
    index = 0;
  }
  ui->langCombo->setCurrentIndex(index);
  connect(ui->langCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int) {
            ConfigModel::saveLocale(ui->langCombo->currentData().toString());
            ui->restartButton->setVisible(true);
          });

  // Show invisibles combo box
  ui->showInvisiblesCheck->setChecked(ConfigModel::showInvisibles());
  connect(ui->showInvisiblesCheck, &QCheckBox::toggled, [=](bool checked) {
    ConfigModel::saveShowInvisibles(checked);
    // todo: Update TextEditView to reflect this setting
  });

  // EOL text
  ui->eolEdit->setText(ConfigModel::endOfLineStr());
  connect(ui->eolEdit, &QLineEdit::textEdited, this,
          [=](const QString& text) { ConfigModel::saveEndOfLineStr(text); });

  // Restart button to apply change
  connect(ui->restartButton, &QPushButton::clicked, this, [=] { SilkApp::restart(); });
}

GeneralSettingsView::~GeneralSettingsView() {
  delete ui;
}
