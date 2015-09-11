#include <memory>
#include <QFile>
#include <QTextStream>
#include <QTextDocument>
#include <QTextBlock>
#include <QFileDialog>
#include <QDebug>

#include "DocumentManager.h"
#include "Window.h"
#include "SilkApp.h"
#include "TabView.h"
#include "core/Document.h"

using core::Document;

const QString DocumentManager::DEFAULT_FILE_NAME = "untitled";

bool DocumentManager::open(const QString& filename) {
  if (TabView* tabView = SilkApp::activeTabView()) {
    return tabView->open(filename) >= 0;
  } else {
    qWarning("active tab view is null");
    return false;
  }
}

bool DocumentManager::save(Document* doc) {
  if (!doc) {
    qWarning("doc is null");
    return false;
  }

  if (doc->path().isEmpty()) {
    QString newFilePath = saveAs(doc);
    return !newFilePath.isEmpty();
  }

  QFile outFile(doc->path());
  if (outFile.open(QIODevice::WriteOnly)) {
    QTextStream out(&outFile);
    out.setCodec(doc->encoding().codec());
    for (int i = 0; i < doc->blockCount(); i++) {
      if (i < doc->blockCount() - 1) {
        out << doc->findBlockByNumber(i).text() << doc->lineSeparator() << flush;
      } else {
        // don't output a new line character in the last block.
        out << doc->findBlockByNumber(i).text();
      }
    }
    return true;
  }

  return false;
}

QString DocumentManager::saveAs(Document* doc) {
  QString filePath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As"), doc->path());
  if (!filePath.isEmpty()) {
    doc->setPath(filePath);
    save(doc);
  }

  return filePath;
}
