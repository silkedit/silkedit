#pragma once

#include <boost/optional.hpp>
#include <QDialog>

#include "core/UniqueObject.h"
#include "core/macros.h"

namespace Ui {
class InputDialog;
}

// QInputDialog doesn't have validation capability. So we created custom InputDialog.
class InputDialog : public QDialog, public core::UniqueObject {
  Q_OBJECT

 public:
  explicit InputDialog(QWidget* parent = 0);
  ~InputDialog();
  DEFAULT_COPY_AND_MOVE(InputDialog)

  Q_INVOKABLE void setLabelText(const QString& label);
  Q_INVOKABLE void setTextValue(const QString& text);
  Q_INVOKABLE void disableOK();
  Q_INVOKABLE void enableOK();
  Q_INVOKABLE boost::optional<QString> show();

 private:
  Ui::InputDialog* ui;

  QString textValue();
  void textChanged(const QString& text);
};

Q_DECLARE_METATYPE(InputDialog*)
