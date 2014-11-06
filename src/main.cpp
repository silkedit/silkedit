#include <QtWidgets>

#include "mainWindow.h"

int main(int argv, char** args) {
  QApplication app(argv, args);

  MainWindow w;
  w.show();
  return app.exec();
}
