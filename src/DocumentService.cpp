#include <memory>
#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <stextedit.h>

#include "DocumentService.h"
#include "MainWindow.h"
#include "API.h"
#include "STabWidget.h"

bool DocumentService::open(const QString& filename) {
  MainWindow* window = API::activeWindow();
  if (window) {
    return window->activeTabWidget()->open(filename) >= 0;
  } else {
    qWarning("m_tabWidget is null");
    return false;
  }
}
