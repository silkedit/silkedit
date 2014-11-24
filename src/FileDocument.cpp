#include "FileDocument.h"

FileDocument::FileDocument(const QString& filename, std::unique_ptr<QTextDocument> document)
    : m_filename(filename), m_document(std::move(document)) {
}
