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
  ~DocumentService() = default;

  bool open(const QString& filename);

 private:
  friend class Singleton<DocumentService>;
  DocumentService() = default;
};
