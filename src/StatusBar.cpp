#include <QMessageBox>

#include "StatusBar.h"
#include "LanguageComboBox.h"
#include "EncodingComboBox.h"
#include "LineSeparatorComboBox.h"
#include "BOMComboBox.h"
#include "Window.h"
#include "TabView.h"
#include "TextEditView.h"
#include "ReloadEncodingDialog.h"
#include "core/Config.h"
#include "core/Theme.h"
#include "core/LanguageParser.h"
#include "core/LineSeparator.h"
#include "core/BOM.h"
#include "core/Util.h"

using core::Encoding;
using core::Config;
using core::Theme;
using core::Language;
using core::LanguageParser;
using core::LanguageProvider;
using core::LineSeparator;
using core::ColorSettings;
using core::BOM;
using core::ColorSettings;
using core::Util;

StatusBar::StatusBar(QMainWindow* window)
    : QStatusBar(window),
      m_langComboBox(new LanguageComboBox),
      m_separatorComboBox(new LineSeparatorComboBox),
      m_encComboBox(new EncodingComboBox),
      m_bomComboBox(new BOMComboBox) {
#ifdef Q_OS_WIN
  // Can't set padding and margin of StatusBar and ComboBox in a stylesheet.
  // As a workaround, setSizeGripEnabled(true) (defualt value) to set a right margin of StatusBar on
  // Mac
  setSizeGripEnabled(false);
#endif

  m_langComboBox->setFocusPolicy(Qt::NoFocus);
  m_separatorComboBox->setFocusPolicy(Qt::NoFocus);
  m_encComboBox->setFocusPolicy(Qt::NoFocus);
  m_bomComboBox->setFocusPolicy(Qt::NoFocus);

  // StatusBar becomes the owner of these widgets
  addPermanentWidget(m_bomComboBox);
  addPermanentWidget(m_separatorComboBox);
  addPermanentWidget(m_encComboBox);
  addPermanentWidget(m_langComboBox);

  connect(&Config::singleton(), &Config::themeChanged, this, &StatusBar::setTheme);
  connect(m_langComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &StatusBar::setActiveTextEditViewLanguage);
  connect(m_encComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &StatusBar::setActiveTextEditViewEncoding);
  connect(m_separatorComboBox,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          &StatusBar::setActiveTextEditViewLineSeparator);
  connect(m_bomComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &StatusBar::setActiveTextEditViewBOM);

  // Set default values
  setTheme(Config::singleton().theme());
  setCurrentLanguage(LanguageProvider::defaultLanguage());
  setEncoding(Encoding::defaultEncoding());
  setLineSeparator(LineSeparator::defaultLineSeparator().separatorStr());
  setBOM(BOM::defaultBOM());
}

void StatusBar::onActiveViewChanged(QWidget*, QWidget* newView) {
  //  qDebug("onActiveViewChanged");
  TextEditView* textEdit = qobject_cast<TextEditView*>(newView);
  if (textEdit) {
    setCurrentLanguage(textEdit->language());
    if (auto enc = textEdit->encoding()) {
      setEncoding(*enc);
    }
    if (auto separator = textEdit->lineSeparator()) {
      setLineSeparator(*separator);
    }
    if (auto bom = textEdit->BOM()) {
      setBOM(*bom);
    }
  } else {
    qDebug() << "active view is not TextEdit";
  }
}

void StatusBar::setActiveTextEditViewLanguage() {
  qDebug() << "currentIndexChanged in langComboBox." << m_langComboBox->currentIndex();
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEditView* editView = qobject_cast<TextEditView*>(tabView->activeView())) {
      qDebug() << "active editView's lang:" << editView->language()->scopeName;
      editView->setLanguage(m_langComboBox->currentData().toString());
    } else {
      qDebug() << "active TextEditView is null";
    }
  } else {
    qDebug() << "active tab widget is null";
  }
}

void StatusBar::setActiveTextEditViewEncoding() {
  qDebug("currentIndexChanged in encComboBox. %d", m_encComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEditView* editView = qobject_cast<TextEditView*>(tabView->activeView())) {
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
    if (TextEditView* editView = qobject_cast<TextEditView*>(tabView->activeView())) {
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

void StatusBar::setActiveTextEditViewBOM() {
  qDebug("currentIndexChanged in bomComboBox. %d", m_bomComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEditView* editView = qobject_cast<TextEditView*>(tabView->activeView())) {
      if (auto bom = editView->BOM()) {
        qDebug("active editView's BOM: %s", qPrintable(bom->name()));
        editView->setBOM(m_bomComboBox->currentBOM());
      }
    } else {
      qDebug("active TextEditView is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}

void StatusBar::setTheme(const Theme* theme) {
  qDebug("StatusBar theme is changed");
  if (!theme) {
    qWarning("theme is null");
    return;
  }

  if (theme->statusBarSettings != nullptr) {
    QString style;
    ColorSettings* statusBarSettings = theme->statusBarSettings.get();

    style = QString("background-color: %1;")
                .arg(Util::qcolorForStyleSheet(statusBarSettings->value("background")));
    style += QString("color: %1;")
                 .arg(Util::qcolorForStyleSheet(statusBarSettings->value("foreground")));
    this->setStyleSheet(QString("StatusBar, StatusBar QComboBox{%1}").arg(style));
  }
}

void StatusBar::setLanguage(const QString& scope) {
  qDebug("setLanguage inStatusBar. scope: %s", qPrintable(scope));
  Language* lang = LanguageProvider::languageFromScope(scope);
  setCurrentLanguage(lang);
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

void StatusBar::setBOM(const BOM& bom) {
  m_bomComboBox->setCurrentBOM(bom);
}
