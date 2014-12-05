#pragma once

#include <unordered_map>
#include <QString>

#include "FileDocument.h"
#include "Singleton.h"
#include "macros.h"
#include "stlSpecialization.h"

class STabWidget;

class DocumentService : public Singleton<DocumentService> {
  DISABLE_COPY_AND_MOVE(DocumentService)
 public:
  static const QString DEFAULT_FILE_NAME;
  ~DocumentService() = default;

  bool open(const QString& filename);
  void save(const QString& path, QTextDocument* doc);
  QString saveAs(const QString& path, QTextDocument* doc);

 private:
  friend class Singleton<DocumentService>;
  DocumentService() = default;
};
