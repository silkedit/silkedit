#include <QPushButton>

#include "InputDialog.h"
#include "ui_InputDialog.h"
#include "HelperProxy.h"

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

void InputDialog::request(InputDialog* view,
                          const QString& method,
                          msgpack::rpc::msgid_t msgId,
                          const msgpack::v1::object&) {
  if (method == "new") {
    InputDialog* dialog = new InputDialog();
    HelperProxy::singleton().sendResponse(dialog->id(), msgpack::type::nil(), msgId);
  } else if (method == "show") {
    view->ui->lineEdit->selectAll();
    int result = view->exec();
    if (result == QDialog::Accepted) {
      QString text = view->textValue();
      std::string textStr = text.toUtf8().constData();
      HelperProxy::singleton().sendResponse(textStr, msgpack::type::nil(), msgId);
    } else {
      HelperProxy::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
    }
  } else {
    qWarning("%s is not supported", qPrintable(method));
    HelperProxy::singleton().sendResponse(msgpack::type::nil(), msgpack::type::nil(), msgId);
  }
}

void InputDialog::notify(InputDialog* view, const QString& method, const msgpack::v1::object& obj) {
  int numArgs = obj.via.array.size;
  if (method == "setLabelText" && numArgs == 2) {
    std::tuple<int, std::string> params;
    obj.convert(&params);
    std::string text = std::get<1>(params);
    view->setLabelText(QString(text.c_str()));
  } else if (method == "setTextValue" && numArgs == 2) {
    std::tuple<int, std::string> params;
    obj.convert(&params);
    std::string text = std::get<1>(params);
    view->setTextValue(QString::fromUtf8(text.c_str()));
  } else if (method == "delete") {
    delete view;
  } else if (method == "disableOK") {
    view->disableOK();
  } else if (method == "enableOK") {
    view->enableOK();
  } else {
    qWarning("%s is not supported", qPrintable(method));
  }
}

void InputDialog::textChanged(const QString& text) {
  std::string type = text.toUtf8().constData();
  std::tuple<int, std::string> params = std::make_tuple(id(), type);
  HelperProxy::singleton().sendNotification("InputDialog.textValueChanged", params);
}
