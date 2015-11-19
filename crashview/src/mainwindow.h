#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

 protected:
  void changeEvent(QEvent* e);

 private slots:

  void on_actionExit_triggered();

  void on_actionOpen_triggered();

 private:
  Ui::MainWindow* ui;

  void showDump(const QString& fileName);
};

#endif  // MAINWINDOW_H
