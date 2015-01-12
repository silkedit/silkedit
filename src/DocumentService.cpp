#include <memory>
#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <QTextBlock>
#include <QFileDialog>
#include <QDebug>

#include "DocumentService.h"
#include "MainWindow.h"
#include "API.h"
#include "STabWidget.h"
#include "Document.h"

const QString DocumentService::DEFAULT_FILE_NAME = "untitled";

bool DocumentService::open(const QString& filename) {
  MainWindow* window = API::activeWindow();
  if (window) {
    return window->activeTabWidget()->open(filename) >= 0;
  } else {
    qWarning("m_tabWidget is null");
    return false;
  }
}

void DocumentService::save(Document* doc) {
  if (doc->path().isEmpty()) {
    doc->setPath(DEFAULT_FILE_NAME);
    saveAs(doc);
    return;
  } else if (!doc) {
    qWarning("document is null");
    return;
  }

  QFile outFile(doc->path());
  if (outFile.open(QIODevice::WriteOnly)) {
    QTextStream out(&outFile);
    for (int i = 0; i < doc->blockCount(); i++) {
      if (i < doc->blockCount() - 1) {
        out << doc->findBlockByNumber(i).text() << endl;
      } else {
        // don't output a new line character in the last block.
        out << doc->findBlockByNumber(i).text();
      }
    }
  }
}

QString DocumentService::saveAs(Document* doc) {
  QString filePath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As"), doc->path());
  if (!filePath.isEmpty()) {
    doc->setPath(filePath);
    save(doc);
  }

  return filePath;
}
