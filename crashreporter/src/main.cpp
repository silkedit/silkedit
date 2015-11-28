#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QLocale>
#include <QDebug>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

#ifdef Q_OS_MAC
  QString path = QCoreApplication::applicationDirPath() + "/../Resources/translations";
  QDir dir(path);
  if( !dir.exists()) {
      path = QApplication::applicationDirPath() + "/../Resources/translations";
  }
#elif defined Q_OS_WIN
  QString path = QApplication::applicationDirPath();
#else
  QString path = "";
#endif
  QTranslator trans;
  QString file = "crashreporter_" + QLocale::system().name();
  bool b = trans.load(file,path);
  if(!b) {
      qDebug() << "No translation file:" << path << "/" << file;
  }
  app.installTranslator(&trans);

  MainWindow wnd;
  wnd.show();

  return app.exec();
}
