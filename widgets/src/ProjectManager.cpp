#include "ProjectManager.h"
#include "ProjectTreeView.h"
#include "Window.h"
#include "TabView.h"

bool ProjectManager::open(const QString& directoryPath) {
  if (!QDir(directoryPath).exists()) {
    qWarning("%s doesn't exist", qPrintable(directoryPath));
    return false;
  }

  Window* window = new Window();
  if (window) {
    window->show();
    return window->openDir(QDir::fromNativeSeparators(directoryPath));
  } else {
    qWarning("active window or its project view is null");
    return false;
  }
}
