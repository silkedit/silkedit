﻿#pragma once

#include <QStatusBar>

#include "core/macros.h"

class TextEditView;
class LanguageComboBox;
class EncodingComboBox;
class LineSeparatorComboBox;
class BOMComboBox;
class QMainWindow;
namespace core {
struct Language;
class Encoding;
class BOM;
}

class StatusBar : public QStatusBar{
  Q_OBJECT
  DISABLE_COPY(StatusBar)

 public:
  explicit StatusBar(QMainWindow* window);
  ~StatusBar() = default;
  DEFAULT_MOVE(StatusBar)

  void onActiveTextEditViewChanged(TextEditView* oldEditView, TextEditView* newEditView);
  void setLanguage(const QString& scope);
  void setEncoding(const core::Encoding& encoding);
  void setLineSeparator(const QString& separator);
  void setBOM(const core::BOM& bom);
  void setActiveTextEditViewLanguage();
  void setActiveTextEditViewEncoding();
  void setActiveTextEditViewLineSeparator();
  void setActiveTextEditViewBOM();

public slots:
  void showMessageWithTimeout(const QString& text, int timeout);

 signals:
  void languageChanged(const QString& scopeName);

 private:
  LanguageComboBox* m_langComboBox;
  LineSeparatorComboBox* m_separatorComboBox;
  EncodingComboBox* m_encComboBox;
  BOMComboBox* m_bomComboBox;

  void setCurrentLanguage(core::Language* lang);
};

Q_DECLARE_METATYPE(StatusBar*)
