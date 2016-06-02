#pragma once

#include <memory>
#include <QWidget>

#include "core/HistoryModel.h"
#include "CustomWidget.h"

namespace Ui {
class Console;
}

namespace core {
class Theme;
}

class Console : public CustomWidget {
  Q_OBJECT

 public:
  explicit Console(QWidget* parent = 0);
  ~Console();

 protected:
  void showEvent(QShowEvent*);
  void hideEvent(QHideEvent* event);

 private:
  std::unique_ptr<Ui::Console> ui;
  core::HistoryModel m_historyModel;

  void runJSCode(const QString& code);
  void setTheme(core::Theme* theme);
};

Q_DECLARE_METATYPE(Console*)
