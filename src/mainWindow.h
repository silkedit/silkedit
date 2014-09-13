#pragma once

#include <QMainWindow>

#include "viEditView.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
  ~MainWindow();

public slots:
  void onModeChanged();

private:
  MainWindow(const MainWindow &);
  MainWindow &operator=(const MainWindow &);

  ViEditView *m_editor;
};
