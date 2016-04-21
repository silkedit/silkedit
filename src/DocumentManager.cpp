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
  if (Window::windows().isEmpty()) {
    if (auto win = Window::createWithNewFile()) {
      win->show();
    }
  }

  if (TabView* tabView = App::instance()->getActiveTabViewOrCreate()) {
    return tabView->open(filename) >= 0;
  } else {
    qWarning("active tab view is null");
    return -1;
  }
}

DocumentManager::DocumentManager() : m_watcher(new QFileSystemWatcher(this)) {
  connect(m_watcher, &QFileSystemWatcher::fileChanged, [=](const QString& path) {
    if (!m_pathDocHash.contains(path)) {
      qCritical() << path << "is not registered";
      return;
    }

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

bool DocumentManager::save(Document* doc, bool beforeClose) {
  if (!doc) {
    qWarning("doc is null");
    return false;
  }

  if (doc->path().isEmpty()) {
    QString newFilePath = saveAs(doc, beforeClose);
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

    if (!beforeClose) {
      // calling addPath immediately still fires fileChanged signal on Windows.
      QTimer::singleShot(0, this, [=] {
        if (doc) {
          m_watcher->addPath(doc->path());
        }
      });
    }

    return true;
  }

  return false;
}

QString DocumentManager::saveAs(Document* doc, bool beforeClose) {
  QString filePath =
      QFileDialog::getSaveFileName(nullptr, QObject::tr("Save As"), doc->path(), QString());
  if (!filePath.isEmpty()) {
    doc->setPath(filePath);
    bool result = save(doc, beforeClose);
    if (!result) {
      qWarning() << "Failed to save" << doc->path();
    }
  }

  return filePath;
}

std::shared_ptr<core::Document> DocumentManager::registerDoc(Document* doc) {
  if (doc) {
    const auto& path = doc->path();
    auto sharedDoc = std::shared_ptr<Document>(doc);

    if (!path.isEmpty()) {
      m_pathDocHash[path] = std::weak_ptr<Document>(sharedDoc);
      m_watcher->addPath(path);
    }
    if (!doc->objectName().isEmpty()) {
      m_objectNameDocHash[doc->objectName()] = std::weak_ptr<Document>(sharedDoc);
    }

    connect(doc, &core::Document::destroyed, [=](QObject* obj) {
      qDebug() << "document (" << path << ") is destroyed.";
      m_pathDocHash.remove(path);
      m_watcher->removePath(path);
      m_objectNameDocHash.remove(obj->objectName());
    });

    return sharedDoc;
  } else {
    return std::shared_ptr<Document>();
  }
}

std::shared_ptr<core::Document> DocumentManager::create(const QString& path) {
  Q_ASSERT(m_watcher);

  if (m_pathDocHash.contains(path)) {
    auto doc = m_pathDocHash[path].lock();
    Q_ASSERT(doc);
    return doc;
  }

  auto doc = Document::create(path);
  return registerDoc(doc);
}

std::shared_ptr<core::Document> DocumentManager::getOrCreate(QSettings& settings) {
  auto doc = Document::create(settings);

  if (!doc->objectName().isEmpty() && m_objectNameDocHash.contains(doc->objectName())) {
    return m_objectNameDocHash[doc->objectName()].lock();
  }

  return registerDoc(doc);
}
