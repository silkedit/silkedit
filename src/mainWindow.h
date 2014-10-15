#pragma once

#include <QMainWindow>
#include <QLineEdit>

#include "vi.h"

class ViEditView;
class ViEngine;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
  ~MainWindow();

public slots:
  void onModeChanged(Mode);
  void cmdLineReturnPressed();
  void cmdLineCursorPositionChanged(int, int);
  void cmdLineTextChanged(const QString &text);

protected:
  bool eventFilter(QObject *obj, QEvent *evnet);

private:
  MainWindow(const MainWindow &);
  MainWindow &operator=(const MainWindow &);

  ViEditView *m_editor;
  ViEngine *m_viEngine;
  QLineEdit *m_cmdLineEdit;
};
