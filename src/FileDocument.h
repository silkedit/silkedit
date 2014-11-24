#pragma once

#include <memory>
#include <QString>
#include <QTextDocument>

#include "macros.h"

class FileDocument {
  DISABLE_COPY(FileDocument)

 public:
  FileDocument(const QString& filename, std::unique_ptr<QTextDocument> document);
  ~FileDocument() = default;
  DEFAULT_MOVE(FileDocument)

  QTextDocument* document() { return m_document.get(); }

 private:
  QString m_filename;
  std::unique_ptr<QTextDocument> m_document;
};
