#include <QPushButton>

#include "InputDialog.h"
#include "ui_InputDialog.h"
#include "Helper.h"

InputDialog::InputDialog(QWidget* parent) : QDialog(parent), ui(new Ui::InputDialog) {
  ui->setupUi(this);

  connect(ui->lineEdit, &QLineEdit::textChanged, this, &InputDialog::textChanged);
}

InputDialog::~InputDialog() {
  delete ui;
}

void InputDialog::setLabelText(const QString& label) {
  ui->label->setText(label);
}

QString InputDialog::textValue() {
  return ui->lineEdit->text();
}

void InputDialog::setTextValue(const QString& text) {
  ui->lineEdit->setText(text);
}

void InputDialog::disableOK() {
  ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
}

void InputDialog::enableOK() {
  ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
}

boost::optional<QString> InputDialog::show() {
  ui->lineEdit->selectAll();
  int result = exec();
  if (result == QDialog::Accepted) {
    return textValue();
  } else {
    return boost::none;
  }
}

void InputDialog::textChanged(const QString& text) {
  std::string type = text.toUtf8().constData();
  std::tuple<int, std::string> params = std::make_tuple(id(), type);
  Helper::singleton().sendNotification("InputDialog.textValueChanged", params);
}
