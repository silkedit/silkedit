#pragma once

#include <memory>
#include <QString>
#include <QObject>
#include <QFileSystemWatcher>

#include "core/macros.h"
#include "core/Singleton.h"
#include "core/Document.h"

class TabView;
namespace core {
class Document;
}

class DocumentManager : public QObject, public core::Singleton<DocumentManager> {
  Q_OBJECT
  DISABLE_COPY_AND_MOVE(DocumentManager)

 public:
  static const QString DEFAULT_FILE_NAME;

  ~DocumentManager() = default;

  bool save(core::Document* doc);
  QString saveAs(core::Document* doc);
  std::shared_ptr<core::Document> create(const QString& path);

public slots:
  int open(const QString& filename);

 private:
  friend class core::Singleton<DocumentManager>;
  DocumentManager();

  QFileSystemWatcher* m_watcher;
  QHash<QString, std::weak_ptr<core::Document>> m_pathDocHash;
};
