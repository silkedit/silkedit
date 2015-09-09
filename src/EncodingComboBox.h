#pragma once

#include <QComboBox>

#include "macros.h"
#include "Encoding.h"

class TextEditView;

class EncodingComboBox : public QComboBox {
  Q_OBJECT
  DISABLE_COPY(EncodingComboBox)

 public:
  EncodingComboBox(QWidget* parent = nullptr);
  ~EncodingComboBox() = default;
  DEFAULT_MOVE(EncodingComboBox)

  void setCurrentEncoding(const Encoding& encoding);
  Encoding currentEncoding();

  bool isAutoDetectSelected();

 private:
};
