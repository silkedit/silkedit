#pragma once

#include <QString>

#include "macros.h"

class TabView;
class Document;

class DocumentService {
  DISABLE_COPY_AND_MOVE(DocumentService)

 public:
  static const QString DEFAULT_FILE_NAME;

  static bool open(const QString& filename);
  static void save(Document* doc);
  static QString saveAs(Document* doc);

 private:
  DocumentService() = delete;
  ~DocumentService() = delete;
};
