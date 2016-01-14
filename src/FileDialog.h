#pragma once

#include <QFileDialog>

#include "core/macros.h"

class FileDialog : public QFileDialog {
  Q_OBJECT
  DISABLE_COPY(FileDialog)

 public:
  Q_INVOKABLE FileDialog(QWidget* parent = 0,
                         const QString& caption = QString(),
                         const QString& directory = QString(),
                         const QString& filter = QString())
      : QFileDialog(parent, caption, directory, filter) {}
  ~FileDialog() = default;
  DEFAULT_MOVE(FileDialog)

 public slots:
  QStringList selectedFiles() const { return QFileDialog::selectedFiles(); }
};
