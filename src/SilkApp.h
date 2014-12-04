#pragma once

#include <QApplication>

#include "macros.h"

class STabWidget;
class TextEditView;

class SilkApp : public QApplication {
  DISABLE_COPY_AND_MOVE(SilkApp)

 public:
  SilkApp(int &argc, char **argv);
  ~SilkApp() = default;
};
