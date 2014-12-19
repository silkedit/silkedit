#pragma once

#include <unordered_map>
#include <QString>

#include "Document.h"
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
  void save(Document* doc);
  QString saveAs(Document* doc);

 private:
  friend class Singleton<DocumentService>;
  DocumentService() = default;
};
