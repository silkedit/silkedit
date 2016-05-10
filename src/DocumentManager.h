#pragma once

#include <memory>
#include <QString>
#include <QObject>
#include <QFileSystemWatcher>
#include <QSettings>

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

  bool save(core::Document* doc, bool beforeClose);
  QString saveAs(core::Document* doc, bool beforeClose);
  std::shared_ptr<core::Document> create(const QString& path);
  std::shared_ptr<core::Document> getOrCreate(QSettings& settings);
  std::shared_ptr<core::Document> find(const QString& objectName);

public slots:
  int open(const QString& filename);

 private:
  QFileSystemWatcher* m_watcher;
  QHash<QString, std::weak_ptr<core::Document>> m_pathDocHash;
  QHash<QString, std::weak_ptr<core::Document>> m_objectNameDocHash;

  friend class core::Singleton<DocumentManager>;
  DocumentManager();

  std::shared_ptr<core::Document> registerDoc(core::Document* doc);
};
