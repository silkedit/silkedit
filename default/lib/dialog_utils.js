'use strict'

const silkedit = require('silkedit');

function showDialog(title, fileMode, options = 0) {
  // On Windows, native dialog sets QApplication::activeWindow() to NULL. We need to store and
  // restore it after closing the dialog.
  // https://bugreports.qt.io/browse/QTBUG-38414
  const activeWindow = silkedit.App.activeWindow();
  const dialog = new silkedit.FileDialog(null, title);
  dialog.fileMode = fileMode;
  dialog.options = options;
  if (dialog.exec()) {
    silkedit.App.setActiveWindow(activeWindow);
    return dialog.selectedFiles();
  }
  return [];
}

module.exports = {
  showFileDialog: (title) => {
    return showDialog(title, silkedit.FileDialog.FileMode.ExistingFiles);
  },
  showDirectoryDialog: (title) => {
    console.log(`silkedit.FileDialog.Option.ShowDirsOnly : ${silkedit.FileDialog.Option.ShowDirsOnly}`);
    return showDialog(title, silkedit.FileDialog.FileMode.Directory, silkedit.FileDialog.Option.ShowDirsOnly);
  },
  showFileAndDirectoryDialog: (title) => {
    return showDialog(title, silkedit.FileDialog.FileMode.AnyFile);
  }
}
