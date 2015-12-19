#pragma once
#include <QMainWindow>
#include <QString>

// Version information is supplied from the build command line.
#ifndef CRASH_APP_VERSION
  // Version information from version.h
  #include "../src/version.h"
  #define CRASH_APP_VERSION VERSION
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

 private slots:
  void on_pushButton_send_clicked();
  void on_pushButton_exit_clicked();

 private:
  Ui::MainWindow* ui;
  QString fileName;
  QString comment;
};
