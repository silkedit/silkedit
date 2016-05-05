#pragma once

#include <QDialog>

#include "core/macros.h"

class TextEdit;
namespace core {
class Encoding;
}

class ReloadEncodingDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ReloadEncodingDialog(TextEdit* textEdit,
                                const core::Encoding& fromEncoding,
                                const core::Encoding& toEncoding,
                                QWidget* parent = 0);
  ~ReloadEncodingDialog() = default;
  DEFAULT_COPY_AND_MOVE(ReloadEncodingDialog)

 private:
  const core::Encoding& m_fromEncoding;
  const core::Encoding& m_toEncoding;
  TextEdit* m_textEdit;

  void reload();
  void convert();
};
