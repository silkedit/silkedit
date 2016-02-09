#pragma once

#include <QWidget>

namespace Ui {
class Console;
}

class Console : public QWidget {
  Q_OBJECT

 public:
  explicit Console(QWidget* parent = 0);
  ~Console();

 private:
  std::unique_ptr<Ui::Console> ui;

  void runJSCode(const QString& code);
};

Q_DECLARE_METATYPE(Console*)
