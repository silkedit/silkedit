#pragma once

#include <QDialog>

#include "macros.h"

class TextEditView;
class Encoding;

class ReloadEncodingDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ReloadEncodingDialog(TextEditView* editView,
                                const Encoding& fromEncoding,
                                const Encoding& toEncoding,
                                QWidget* parent = 0);
  ~ReloadEncodingDialog() = default;
  DEFAULT_COPY_AND_MOVE(ReloadEncodingDialog)

 private:
  const Encoding& m_fromEncoding;
  const Encoding& m_toEncoding;
  TextEditView* m_editView;

  void reload();
  void convert();
};
