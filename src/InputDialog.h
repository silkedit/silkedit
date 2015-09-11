#pragma once

#include <QDialog>

#include "UniqueObject.h"
#include "core/macros.h"

namespace Ui {
class InputDialog;
}

// QInputDialog doesn't have validation capability. So we created custom InputDialog.
class InputDialog : public QDialog, public UniqueObject<InputDialog> {
  Q_OBJECT

 public:
  explicit InputDialog(QWidget* parent = 0);
  ~InputDialog();
  DEFAULT_COPY_AND_MOVE(InputDialog)

  void setLabelText(const QString& label);
  QString textValue();
  void setTextValue(const QString& text);
  void disableOK();
  void enableOK();

 protected:
  friend struct UniqueObject<InputDialog>;

  static void request(InputDialog* view,
                      const QString& method,
                      msgpack::rpc::msgid_t msgId,
                      const msgpack::object& obj);
  static void notify(InputDialog* view, const QString& method, const msgpack::object& obj);

 private:
  Ui::InputDialog* ui;

  void textChanged(const QString& text);
};
