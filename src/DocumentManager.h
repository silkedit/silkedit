#pragma once

#include <QString>

#include "macros.h"

class TabView;
class Document;

class DocumentManager {
  DISABLE_COPY_AND_MOVE(DocumentManager)

 public:
  static const QString DEFAULT_FILE_NAME;

  static bool open(const QString& filename);
  static bool save(Document* doc);
  static QString saveAs(Document* doc);

 private:
  DocumentManager() = delete;
  ~DocumentManager() = delete;
};