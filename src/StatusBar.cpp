#include <QMessageBox>

#include "StatusBar.h"
#include "LanguageComboBox.h"
#include "EncodingComboBox.h"
#include "LineSeparatorComboBox.h"
#include "BOMComboBox.h"
#include "Window.h"
#include "TabView.h"
#include "TextEdit.h"
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
          this, &StatusBar::setActiveTextEditLanguage);
  connect(m_encComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &StatusBar::setActiveTextEditEncoding);
  connect(m_separatorComboBox,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          &StatusBar::setActiveTextEditLineSeparator);
  connect(m_bomComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &StatusBar::setActiveTextEditBOM);

  // Set default values
  setTheme(Config::singleton().theme());
  if (auto lang = LanguageProvider::defaultLanguage()) {
    setCurrentLanguage(lang->name());
  }
  setEncoding(Encoding::defaultEncoding());
  setLineSeparator(LineSeparator::defaultLineSeparator().separatorStr());
  setBOM(BOM::defaultBOM());
}

void StatusBar::onActiveViewChanged(QWidget*, QWidget* newView) {
  //  qDebug("onActiveViewChanged");
  TextEdit* textEdit = qobject_cast<TextEdit*>(newView);
  if (textEdit) {
    if (textEdit->language()) {
      setCurrentLanguage(textEdit->language()->name());
    }
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

void StatusBar::setActiveTextEditLanguage() {
  qDebug() << "currentIndexChanged in langComboBox." << m_langComboBox->currentIndex();
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEdit* textEdit = qobject_cast<TextEdit*>(tabView->activeView())) {
      qDebug() << "active textEdit's lang:" << textEdit->language()->scopeName;
      textEdit->setLanguage(m_langComboBox->currentData().toString());
    } else {
      qDebug() << "active TextEdit is null";
    }
  } else {
    qDebug() << "active tab widget is null";
  }
}

void StatusBar::setActiveTextEditEncoding() {
  qDebug("currentIndexChanged in encComboBox. %d", m_encComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEdit* textEdit = qobject_cast<TextEdit*>(tabView->activeView())) {
      if (boost::optional<Encoding> oldEncoding = textEdit->encoding()) {
        QString fromEncodingName = oldEncoding->name();
        qDebug("active textEdit's encoding: %s", qPrintable(fromEncodingName));
        if (m_encComboBox->isAutoDetectSelected()) {
          if (textEdit->path().isEmpty()) {
            m_encComboBox->setCurrentEncoding(*oldEncoding);
            return;
          }

          if (textEdit->document()->isModified()) {
            int ret = QMessageBox::question(this, "",
                                            tr("Current document is changed. This change will be "
                                               "lost after reloading. Do you want to continue?"));
            if (ret == QMessageBox::No) {
              m_encComboBox->setCurrentEncoding(*oldEncoding);
              return;
            }
          }
          textEdit->document()->reload();
          m_encComboBox->setCurrentEncoding(textEdit->document()->encoding());
        } else {
          auto newEncoding = m_encComboBox->currentEncoding();
          if (newEncoding != *oldEncoding) {
            // If the document is empty or has no file path, just change current document's
            // encoding
            bool isEmptyDoc = textEdit->document() && textEdit->document()->isEmpty();
            bool hasNoPath = textEdit->document() && textEdit->document()->path().isEmpty();
            if (isEmptyDoc || hasNoPath) {
              textEdit->document()->setEncoding(newEncoding);
            } else {
              // Show the dialog to choose Reload or Convert
              ReloadEncodingDialog dialog(textEdit, *oldEncoding, newEncoding);
              int ret = dialog.exec();
              if (ret == QDialog::Rejected) {
                m_encComboBox->setCurrentEncoding(*oldEncoding);
              }
            }
          }
        }
      }
    } else {
      qDebug("active TextEdit is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}

void StatusBar::setActiveTextEditLineSeparator() {
  qDebug("currentIndexChanged in separatorComboBox. %d", m_separatorComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEdit* textEdit = qobject_cast<TextEdit*>(tabView->activeView())) {
      if (auto separator = textEdit->lineSeparator()) {
        qDebug("active textEdit's line separator: %s", qPrintable(*separator));
        textEdit->setLineSeparator(m_separatorComboBox->currentLineSeparator());
      }
    } else {
      qDebug("active TextEdit is null");
    }
  } else {
    qDebug("active tab widget is null");
  }
}

void StatusBar::setActiveTextEditBOM() {
  qDebug("currentIndexChanged in bomComboBox. %d", m_bomComboBox->currentIndex());
  TabView* tabView = static_cast<Window*>(window())->activeTabView();
  if (tabView) {
    if (TextEdit* textEdit = qobject_cast<TextEdit*>(tabView->activeView())) {
      if (auto bom = textEdit->BOM()) {
        qDebug("active textEdit's BOM: %s", qPrintable(bom->name()));
        textEdit->setBOM(m_bomComboBox->currentBOM());
      }
    } else {
      qDebug("active TextEdit is null");
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
  if (auto lang = LanguageProvider::languageFromScope(scope)) {
    setCurrentLanguage(lang->name());
  }
}

void StatusBar::setCurrentLanguage(const QString& langName) {
  if (!langName.isEmpty()) {
    int idx = m_langComboBox->findText(langName);
    if (idx >= 0) {
      m_langComboBox->setCurrentIndex(idx);
    } else {
      qDebug() << "lang:" << langName << "is not registered.";
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
