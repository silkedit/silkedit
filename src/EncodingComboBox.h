#pragma once

#include <QComboBox>

#include "macros.h"

class Encoding;

class EncodingComboBox : public QComboBox {
  DISABLE_COPY(EncodingComboBox)

 public:
  EncodingComboBox(QWidget* parent = nullptr);
  ~EncodingComboBox() = default;
  DEFAULT_MOVE(EncodingComboBox)

  void setCurrentEncoding(const Encoding& encoding);

 private:
};
