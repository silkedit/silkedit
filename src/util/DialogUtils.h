#pragma once

#include <list>
#include <string>
#include <QString>
#include <QFileDialog>

#include "macros.h"

class DialogUtils {
  DISABLE_COPY_AND_MOVE(DialogUtils)

 public:
  enum class MODE { FileAndDirectory, Files, Directory };

  static std::list<std::string> showDialog(const QString& caption, MODE mode);

 private:
  static std::list<std::string> showDialogImpl(const QString& caption,
                                        QFileDialog::FileMode fileMode,
                                        QFileDialog::Options options = 0);

  DialogUtils() = delete;
  ~DialogUtils() = delete;
};
