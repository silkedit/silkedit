#pragma once

#include <memory>
#include <QMainWindow>

#include "macros.h"
#include "ViEditView.h"
#include "ViEngine.h"

class MainWindow : public QMainWindow {
  Q_OBJECT
  DISABLE_COPY(MainWindow)

 public:
  MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);
  ~MainWindow() = default;
  DEFAULT_MOVE(MainWindow)

 private:
  std::unique_ptr<ViEditView> m_editor;
  std::unique_ptr<ViEngine> m_viEngine;
};
