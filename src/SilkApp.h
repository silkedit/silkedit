#pragma once

#include <QApplication>

#include "macros.h"

class TabBar;

class SilkApp : public QApplication {
  DISABLE_COPY_AND_MOVE(SilkApp)

 public:
  static TabBar* tabBatAt(int x, int y);

  SilkApp(int& argc, char** argv);
  ~SilkApp() = default;

  // accessor
  QString initialFile() { return m_initialFile; }

  bool event(QEvent*) override;

 private:
  QString m_initialFile;
};
