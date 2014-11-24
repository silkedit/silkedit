#pragma once

#include <unordered_map>
#include <QString>

#include "FileDocument.h"
#include "Singleton.h"
#include "macros.h"
#include "stlSpecialization.h"

class LayoutView;

class DocumentService : public Singleton<DocumentService> {
  DISABLE_COPY_AND_MOVE(DocumentService)
 public:
  ~DocumentService() = default;

  bool open(const QString& filename);
  void setLayoutView(LayoutView* layoutView) { m_layoutView = layoutView; }

 private:
  friend class Singleton<DocumentService>;
  DocumentService();

  std::unordered_map<QString, FileDocument> m_documents;
  LayoutView* m_layoutView;
};
