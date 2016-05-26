#pragma once

#include <memory>
#include <QWidget>

#include "core/HistoryModel.h"

namespace Ui {
class Console;
}

class Console : public QWidget {
  Q_OBJECT

 public:
  explicit Console(QWidget* parent = 0);
  ~Console();

 protected:
  void showEvent(QShowEvent*);

 private:
  std::unique_ptr<Ui::Console> ui;
  core::HistoryModel m_historyModel;

  void runJSCode(const QString& code);
};

Q_DECLARE_METATYPE(Console*)
