#pragma once

#include <QDialog>

#include "macros.h"

class TextEditView;
namespace core {
class Encoding;
}

class ReloadEncodingDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ReloadEncodingDialog(TextEditView* editView,
                                const core::Encoding& fromEncoding,
                                const core::Encoding& toEncoding,
                                QWidget* parent = 0);
  ~ReloadEncodingDialog() = default;
  DEFAULT_COPY_AND_MOVE(ReloadEncodingDialog)

 private:
  const core::Encoding& m_fromEncoding;
  const core::Encoding& m_toEncoding;
  TextEditView* m_editView;

  void reload();
  void convert();
};
