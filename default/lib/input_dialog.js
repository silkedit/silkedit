'use strict'

const silkedit = require('silkedit');

function validateInput(validateFunc, text, buttonBox) {
  // Validate input
  console.log('validateInput');
  if (validateFunc != null) {
    validateFunc(text, (result) => {
      buttonBox.button(silkedit.DialogButtonBox.StandardButton.Ok).setEnabled(result);
    });
  }
}
  
// class InputDialog
function InputDialog(labelText, initialText, validateFunc) {
  this.dialog = new silkedit.Dialog;
  this.dialog.resize(500, 108);
  this.rootLayout = new silkedit.VBoxLayout;
  this.validateFunc = validateFunc;
  const label = new silkedit.Label(labelText);
  this.lineEdit = new silkedit.LineEdit(initialText);
  const buttonBox = new silkedit.DialogButtonBox;
  buttonBox.on('accepted', () => this.dialog.accept());
  buttonBox.on('rejected', () => this.dialog.reject());
    
  if (validateFunc != null) {
    this.lineEdit.on('textChanged', (text) => validateInput(validateFunc, text, buttonBox));
  }

  buttonBox.orientation = silkedit.Orientation.Horizontal;
  buttonBox.standardButtons = silkedit.DialogButtonBox.StandardButton.Ok | silkedit.DialogButtonBox.StandardButton.Cancel;
  this.rootLayout.addWidget(label);
  this.rootLayout.addWidget(this.lineEdit);
  this.rootLayout.addWidget(buttonBox);
  this.dialog.setLayout(this.rootLayout);
}

InputDialog.prototype.show = function() {
  this.lineEdit.selectAll();
  this.lineEdit.emit('textChanged', this.lineEdit.text);
  const result = this.dialog.exec();
  return result === silkedit.Dialog.DialogCode.Accepted ? this.lineEdit.text : null;
}

module.exports = InputDialog;