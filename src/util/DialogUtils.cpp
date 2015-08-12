#include <QApplication>
#include <QFileDialog>

#include "DialogUtils.h"
#include "Util.h"

std::list<std::string> DialogUtils::showDialog(const QString& caption, DialogUtils::MODE mode) {
  switch (mode) {
    case MODE::FileAndDirectory:
      return showDialogImpl(caption, QFileDialog::AnyFile);
    case MODE::Files:
      return showDialogImpl(caption, QFileDialog::ExistingFiles);
    case MODE::Directory:
      return showDialogImpl(caption, QFileDialog::Directory, QFileDialog::ShowDirsOnly);
    default:
      qWarning("invalid mode: %d", static_cast<int>(mode));
      return std::list<std::string>();
  }
}

std::list<std::string> DialogUtils::showDialogImpl(const QString& caption,
                                                   QFileDialog::FileMode fileMode,
                                                   QFileDialog::Options options) {
  // On Windows, native dialog sets QApplication::activeWindow() to NULL. We need to store and
  // restore it after closing the dialog.
  // https://bugreports.qt.io/browse/QTBUG-38414
  QWidget* activeWindow = QApplication::activeWindow();
  QFileDialog dialog(nullptr, caption);
  dialog.setFileMode(fileMode);
  dialog.setOptions(options);
  if (dialog.exec()) {
    QApplication::setActiveWindow(activeWindow);
    QStringList paths;
    foreach (const QString& path, dialog.selectedFiles()) {
      paths.append(QDir::toNativeSeparators(path));
    }
    return Util::toStdStringList(paths);
  }
  return std::list<std::string>();
}
