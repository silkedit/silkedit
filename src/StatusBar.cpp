#include <QMessageBox>

#include "StatusBar.h"
#include "LanguageComboBox.h"
#include "EncodingComboBox.h"
#include "LineSeparatorComboBox.h"
#include "Window.h"
#include "TabView.h"
#include "TextEditView.h"
#include "ReloadEncodingDialog.h"
#include "core/LanguageParser.h"
#include "core/LineSeparator.h"

using core::Encoding;
using core::Language;
using core::LanguageParser;
using core::LanguageProvider;
using core::LineSeparator;

StatusBar::StatusBar(QMainWindow* window)
    : QStatusBar(window),
      m_langComboBox(new LanguageComboBox),
      m_separatorComboBox(new LineSeparatorComboBox),
      m_encComboBox(new EncodingComboBox) {
  // StatusBar becomes the owner of these widgets
  addPermanentWidget(m_separatorComboBox);
  addPermanentWidget(m_encComboBox);
  addPermanentWidget(m_langComboBox);

  connect(m_langComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &StatusBar::setActiveTextEditViewLanguage);
  connect(m_encComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &StatusBar::setActiveTextEditViewEncoding);
  connect(m_separatorComboBox,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          &StatusBar::setActiveTextEditViewLineSeparator);

  // Set default values
  setCurrentLanguage(LanguageProvider::defaultLanguage());
  setEncoding(Encoding::defaultEncoding());
  setLineSeparator(LineSeparator::defaultLineSeparator().separatorStr());
}

void StatusBar::onActiveTextEditViewChanged(TextEditView*, TextEditView* newEditView) {
  qDebug("onActiveTextEditViewChanged");
  if (newEditView) {
    setCurrentLanguage(newEditView->language());
    if (auto enc = newEditView->encoding()) {
      setEncoding(*enc);
    }
    if (auto separator = newEditView->lineSeparator()) {
      setLineSeparator(*separator);
    }
  } else {
    qDebug("newEditView is null");
  }
}

void StatusBar::setActiveTextEditViewLanguage() {
  qDebug("currentIndexChanged in langComboBox. %d", m_langComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEditView* editView = tabView->activeEditView()) {
      qDebug("active editView's lang: %s", qPrintable(editView->language()->scopeName));
      editView->setLanguage(m_langComboBox->currentData().toString());
    } else {
      qDebug("active TextEditView is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}

void StatusBar::setActiveTextEditViewEncoding() {
  qDebug("currentIndexChanged in encComboBox. %d", m_encComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEditView* editView = tabView->activeEditView()) {
      if (boost::optional<Encoding> oldEncoding = editView->encoding()) {
        QString fromEncodingName = oldEncoding->name();
        qDebug("active editView's encoding: %s", qPrintable(fromEncodingName));
        if (m_encComboBox->isAutoDetectSelected()) {
          if (editView->path().isEmpty()) {
            m_encComboBox->setCurrentEncoding(*oldEncoding);
            return;
          }

          if (editView->document()->isModified()) {
            int ret = QMessageBox::question(this, "",
                                            tr("Current document is changed. This change will be "
                                               "lost after reloading. Do you want to continue?"));
            if (ret == QMessageBox::No) {
              m_encComboBox->setCurrentEncoding(*oldEncoding);
              return;
            }
          }
          editView->document()->reload();
          m_encComboBox->setCurrentEncoding(editView->document()->encoding());
        } else {
          auto newEncoding = m_encComboBox->currentEncoding();
          if (newEncoding != *oldEncoding) {
            // If the document is empty or has no file path, just change current document's
            // encoding
            bool isEmptyDoc = editView->document() && editView->document()->isEmpty();
            bool hasNoPath = editView->document() && editView->document()->path().isEmpty();
            if (isEmptyDoc || hasNoPath) {
              editView->document()->setEncoding(newEncoding);
            } else {
              // Show the dialog to choose Reload or Convert
              ReloadEncodingDialog dialog(editView, *oldEncoding, newEncoding);
              int ret = dialog.exec();
              if (ret == QDialog::Rejected) {
                m_encComboBox->setCurrentEncoding(*oldEncoding);
              }
            }
          }
        }
      }
    } else {
      qDebug("active TextEditView is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}

void StatusBar::setActiveTextEditViewLineSeparator() {
  qDebug("currentIndexChanged in separatorComboBox. %d", m_separatorComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEditView* editView = tabView->activeEditView()) {
      if (auto separator = editView->lineSeparator()) {
        qDebug("active editView's line separator: %s", qPrintable(*separator));
        editView->setLineSeparator(m_separatorComboBox->currentLineSeparator());
      }
    } else {
      qDebug("active TextEditView is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}

void StatusBar::setLanguage(const QString& scope) {
  qDebug("setLanguage inStatusBar. scope: %s", qPrintable(scope));
  Language* lang = LanguageProvider::languageFromScope(scope);
  setCurrentLanguage(lang);
}

void StatusBar::request(StatusBar*, const QString&, msgpack::rpc::msgid_t, const msgpack::object&) {
}

void StatusBar::notify(StatusBar* view, const QString& method, const msgpack::object& obj) {
  int numArgs = obj.via.array.size;
  if (method == "showMessage") {
    if (numArgs == 2) {
      std::tuple<int, std::string> params;
      obj.convert(&params);
      std::string message = std::get<1>(params);
      view->showMessage(QString::fromUtf8(message.c_str()));
    }
  } else if (method == "clearMessage") {
    view->clearMessage();
  }
}

void StatusBar::setCurrentLanguage(Language* lang) {
  if (lang) {
    int idx = m_langComboBox->findText(lang->name());
    if (idx >= 0) {
      m_langComboBox->setCurrentIndex(idx);
    } else {
      qDebug("lang: %s is not registered.", qPrintable(lang->name()));
    }
  }
}

void StatusBar::setEncoding(const Encoding& encoding) {
  m_encComboBox->setCurrentEncoding(encoding);
}

void StatusBar::setLineSeparator(const QString& separator) {
  m_separatorComboBox->setCurrentLineSeparator(separator);
}
