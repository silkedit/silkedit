#pragma once

#include "core/macros.h"
#include "core/Encoding.h"
#include "SComboBox.h"

class EncodingComboBox : public SComboBox {
  Q_OBJECT
  DISABLE_COPY(EncodingComboBox)

 public:
  EncodingComboBox(QWidget* parent = nullptr);
  ~EncodingComboBox() = default;
  DEFAULT_MOVE(EncodingComboBox)

  void setCurrentEncoding(const core::Encoding& encoding);
  core::Encoding currentEncoding();

  bool isAutoDetectSelected();

 private:
};
