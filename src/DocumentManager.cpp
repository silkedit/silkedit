#include <QFile>
#include <QTextStream>
#include <QTextBlock>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>

#include "DocumentManager.h"
#include "App.h"
#include "TabView.h"
#include "Window.h"
#include "OpenRecentItemManager.h"
#include "core/Document.h"

using core::Document;

const QString DocumentManager::DEFAULT_FILE_NAME = "untitled";

int DocumentManager::open(const QString& filename) {
  if (TabView* tabView = App::instance()->activeTabView()) {
    return tabView->open(filename) >= 0;
  } else {
    qWarning("active tab view is null");
    return -1;
  }
}

DocumentManager::DocumentManager() : m_watcher(new QFileSystemWatcher(this)) {
  connect(m_watcher, &QFileSystemWatcher::fileChanged, [=](const QString& path) {
    Q_ASSERT(m_pathDocHash.contains(path));
    auto doc = m_pathDocHash[path].lock();
    Q_ASSERT(doc);
    if (QFileInfo::exists(path)) {
      qDebug() << "file is changed";
      auto result = QMessageBox::NoButton;
      if (doc->isModified()) {
        result = QMessageBox::question(
            nullptr, "", tr("%1 \n\nhas changed on disk. Do you want to reload?").arg(path));
      }

      if (!doc->isModified() || result == QMessageBox::Yes) {
        doc->reload(doc->encoding());
      }
    } else {
      qDebug() << "file is removed";
      OpenRecentItemManager::singleton().removeOpenRecentItem(path);
      auto result = QMessageBox::question(
          nullptr, "",
          tr("%1 \n\nhas been removed on disk. Do you want to close its tab?").arg(path));
      if (result == QMessageBox::Yes) {
        Window::closeTabIncludingDoc(doc.get());
      }
    }
  });
}

bool DocumentManager::save(Document* doc) {
  if (!doc) {
    qWarning("doc is null");
    return false;
  }

  if (!doc->isModified()) {
    return false;
  }

  if (doc->path().isEmpty()) {
    QString newFilePath = saveAs(doc);
    return !newFilePath.isEmpty();
  }

  QFile outFile(doc->path());
  if (outFile.open(QIODevice::WriteOnly)) {
    // remove path from QFileSystemWatcher to prevent reloading after save
    m_watcher->removePath(doc->path());

    QTextStream out(&outFile);
    out.setCodec(doc->encoding().codec());
    out.setGenerateByteOrderMark(doc->bom().bomSwitch());
    for (int i = 0; i < doc->blockCount(); i++) {
      if (i < doc->blockCount() - 1) {
        out << doc->findBlockByNumber(i).text() << doc->lineSeparator() << flush;
      } else {
        // don't output a new line character in the last block.
        out << doc->findBlockByNumber(i).text();
      }
    }

    // calling addPath immediately still fires fileChanged signal on Windows.
    QTimer::singleShot(0, this, [=] {
      m_watcher->addPath(doc->path());
    });

    return true;
  }

  return false;
}

QString DocumentManager::saveAs(Document* doc) {
  QString filePath =
      QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As"), doc->path(), QString());
  if (!filePath.isEmpty()) {
    doc->setPath(filePath);
    save(doc);
  }

  return filePath;
}

std::shared_ptr<core::Document> DocumentManager::create(const QString& path) {
  Q_ASSERT(m_watcher);

  if (m_pathDocHash.contains(path)) {
    auto doc = m_pathDocHash[path].lock();
    Q_ASSERT(doc);
    doc->setModified(doc->isModified());
    return doc;
  }

  auto doc = Document::create(path);
  if (doc) {
    connect(doc, &core::Document::destroyed, [=](QObject*) {
      qDebug() << "document (" << path << ") is destroyed.";
      m_pathDocHash.remove(path);
      m_watcher->removePath(path);
    });
    auto sharedDoc = std::shared_ptr<Document>(doc);
    m_pathDocHash[path] = std::weak_ptr<Document>(sharedDoc);
    m_watcher->addPath(path);
    // calling setModified(false) on a document and then inserting text causes the signal to get
    // emitted
    doc->setModified(false);
    return sharedDoc;
  } else {
    return std::shared_ptr<Document>();
  }
}
