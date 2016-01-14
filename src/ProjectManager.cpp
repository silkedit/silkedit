#include "ProjectManager.h"
#include "ProjectTreeView.h"
#include "Window.h"
#include "TabView.h"

bool ProjectManager::open(const QString& dirName) {
  if (!QDir(dirName).exists()) {
    qWarning("%s doesn't exist", qPrintable(dirName));
    return false;
  }

  Window* window = Window::create();
  if (window) {
    window->show();
    return window->openDir(dirName);
  } else {
    qWarning("active window or its project view is null");
    return false;
  }
}
