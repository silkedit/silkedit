#pragma once

#include <QApplication>

#include "macros.h"

class STabWidget;
class TextEditView;

class SilkApp : public QApplication {
  DISABLE_COPY_AND_MOVE(SilkApp)

 public:
  SilkApp(int& argc, char** argv);
  ~SilkApp() = default;

  // accessor
  QString initialFile() { return m_initialFile; }

  bool event(QEvent *) override;

private:
  QString m_initialFile;
};
