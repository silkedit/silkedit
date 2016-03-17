#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "ReloadEncodingDialog.h"
#include "core/Encoding.h"
#include "TextEdit.h"

using core::Encoding;

ReloadEncodingDialog::ReloadEncodingDialog(TextEdit* textEdit,
                                           const Encoding& fromEncoding,
                                           const Encoding& toEncoding,
                                           QWidget* parent)
    : QDialog(parent),
      m_fromEncoding(fromEncoding),
      m_toEncoding(toEncoding),
      m_textEdit(textEdit) {
  QPushButton* cancelBtn = new QPushButton(tr("&Cancel"));
  QPushButton* reloadBtn = new QPushButton(tr("&Reload"));
  QPushButton* convertBtn = new QPushButton(tr("&Convert"));
  auto buttonBox = new QDialogButtonBox(this);
  // The button box takes ownership of the button
  buttonBox->addButton(reloadBtn, QDialogButtonBox::ActionRole);
  buttonBox->addButton(convertBtn, QDialogButtonBox::ActionRole);
  buttonBox->addButton(cancelBtn, QDialogButtonBox::ResetRole);
  auto label = new QLabel(
      tr("Reload: reload current file from disk in %1\nConvert: convert current text in %1")
          .arg(toEncoding.name()),
      this);
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(label);
  layout->addWidget(buttonBox, 0, Qt::AlignRight);
  setLayout(layout);
  setWindowTitle(tr("%1: Reload or Convert to %2").arg(fromEncoding.name()).arg(toEncoding.name()));
  setFixedHeight(sizeHint().height());

  connect(cancelBtn, &QPushButton::clicked, this, &ReloadEncodingDialog::close);
  connect(reloadBtn, &QPushButton::clicked, this, &ReloadEncodingDialog::reload);
  connect(convertBtn, &QPushButton::clicked, this, &ReloadEncodingDialog::convert);
}

void ReloadEncodingDialog::reload() {
  if (!m_textEdit || !m_textEdit->document()) {
    reject();
    return;
  }

  if (m_textEdit->document()->isModified()) {
    int ret = QMessageBox::question(this, "", tr("Current document is changed. This change will be "
                                                 "lost after reloading. Do you want to continue?"));
    if (ret == QMessageBox::No) {
      reject();
      return;
    }
  }

  m_textEdit->document()->reload(m_toEncoding);
  accept();
}

void ReloadEncodingDialog::convert() {
  if (m_textEdit && m_textEdit->document()) {
    m_textEdit->document()->setEncoding(m_toEncoding);
    accept();
  } else {
    reject();
  }
}
