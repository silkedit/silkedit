#pragma once

#include <QComboBox>

#include "core/macros.h"
#include "core/Encoding.h"

class TextEditView;

class EncodingComboBox : public QComboBox {
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
