#include <memory>
#include <stextedit.h>
#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <QTextBlock>

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

void DocumentService::save(const QString &path, QTextDocument *doc)
{
  if (path.isEmpty()) {
    qWarning("path is empty");
    return;
  } else if (!doc) {
    qWarning("document is null");
    return;
  }

  QFile outFile(path);
  if (outFile.open(QIODevice::WriteOnly)) {
    QTextStream out(&outFile);
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
      out << it.text() << endl;
    }
  }
}
