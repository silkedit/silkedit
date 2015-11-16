#pragma once

#include <list>
#include <string>
#include <QString>
#include <QFileDialog>

#include "core/macros.h"

class DialogUtils {
  DISABLE_COPY_AND_MOVE(DialogUtils)

 public:
  enum class MODE { FileAndDirectory, Files, Directory };

  static QStringList showDialog(const QString& caption, MODE mode);

 private:
  static QStringList showDialogImpl(const QString& caption,
                                    QFileDialog::FileMode fileMode,
                                    QFileDialog::Options options = 0);

  DialogUtils() = delete;
  ~DialogUtils() = delete;
};
