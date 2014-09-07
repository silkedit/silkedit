#include <QtWidgets>

#include "viEditView.h"

int main(int argv, char **args) {
  QApplication app(argv, args);

  ViEditView editor;
  editor.setWindowTitle(QObject::tr("Code Editor Example"));
  editor.show();

  return app.exec();
}
