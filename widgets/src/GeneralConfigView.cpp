#include "GeneralConfigView.h"
#include "ui_GeneralConfigView.h"
#include "App.h"
#include "ConfigDialog.h"
#include "core/ThemeManager.h"
#include "core/Config.h"

using core::ThemeManager;
using core::Theme;
using core::Config;

GeneralConfigView::GeneralConfigView(QWidget* parent)
    : CustomWidget(parent), ui(new Ui::GeneralConfigView) {
  ui->setupUi(this);
  addTargetObjects(QObjectList{ui->themeLabel, ui->fontLayout, ui->indentUsingSpacesCheck,
                               ui->tabWidthLabel, ui->langLabel, ui->showInvisiblesCheck,
                               ui->eolLabel});
  ui->restartButton->setVisible(false);

  // Theme combo box
  ui->themeCombo->addItems(ThemeManager::sortedThemeNames());
  if (Config::singleton().theme()) {
    ui->themeCombo->setCurrentText(Config::singleton().theme()->name);
  }
  connect(ui->themeCombo,
          static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
          [=](const QString& text) {
            //            qDebug("themeSelected: %s", qPrintable(text));
            Theme* theme = ThemeManager::theme(text);
            Config::singleton().setTheme(theme);
          });

  // Font combo box
  ui->fontComboBox->setCurrentFont(Config::singleton().font());
  connect(ui->fontComboBox, &QFontComboBox::currentFontChanged,
          [=](const QFont& font) { Config::singleton().setFont(font); });

  // Font size spin box
  ui->fontSizeSpin->setValue(Config::singleton().font().pointSize());
  connect(ui->fontSizeSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [=](int value) {
            QFont font = Config::singleton().font();
            font.setPointSize(value);
            Config::singleton().setFont(font);
          });

  // Indent using spaces checkbox
  ui->indentUsingSpacesCheck->setChecked(Config::singleton().indentUsingSpaces());
  connect(ui->indentUsingSpacesCheck, &QCheckBox::toggled,
          [=](bool checked) { Config::singleton().setIndentUsingSpaces(checked); });

  // Tab width spin box
  ui->tabWidthSpin->setValue(Config::singleton().tabWidth());
  connect(ui->tabWidthSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [=](int value) { Config::singleton().setTabWidth(value); });

  // Language combo box
  ui->langCombo->addItem(tr("<System Language>"), "system");
  ui->langCombo->addItem(tr("English"), "en");
  ui->langCombo->addItem(tr("Japanese"), "ja");
  int index = ui->langCombo->findData(Config::singleton().locale());
  if (index == -1) {
    index = 0;
  }
  ui->langCombo->setCurrentIndex(index);
  connect(ui->langCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int) {
            Config::singleton().setLocale(ui->langCombo->currentData().toString());
            ui->restartButton->setVisible(true);
          });

  // Show invisibles combo box
  ui->showInvisiblesCheck->setChecked(Config::singleton().showInvisibles());
  connect(ui->showInvisiblesCheck, &QCheckBox::toggled,
          [=](bool checked) { Config::singleton().setShowInvisibles(checked); });

  // EOL text
  ui->eolEdit->setText(Config::singleton().endOfLineStr());
  connect(ui->eolEdit, &QLineEdit::textEdited, this,
          [=](const QString& text) { Config::singleton().setEndOfLineStr(text); });

  // Restart button to apply change
  connect(ui->restartButton, &QPushButton::clicked, this, [] {
    ConfigDialog::closeDialog();
    App::instance()->restart();
  });
}

GeneralConfigView::~GeneralConfigView() {
  delete ui;
}
