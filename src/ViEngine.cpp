#include <memory>
#include <QStatusBar>

#include "MainWindow.h"
#include "ViEngine.h"
#include "TextEditView.h"
#include "CommandService.h"
#include "ContextService.h"
#include "KeymapService.h"
#include "ModeContext.h"
#include "commands/ChangeModeCommand.h"

ViEngine::ViEngine(TextEditView* textEditView, MainWindow* mainWindow, QObject* parent)
    : QObject(parent),
      m_mode(Mode::CMD),
      m_textEditView(textEditView),
      m_mainWindow(mainWindow),
      m_repeatCount(0),
      m_cmdLineEdit(new QLineEdit()),
      m_isEnabled(false) {
}

void ViEngine::processExCommand(const QString&) {
  setMode(Mode::CMD);
}

void ViEngine::enable() {
  std::unique_ptr<ChangeModeCommand> changeModeCmd(new ChangeModeCommand(this));
  CommandService::singleton().add(std::move(changeModeCmd));

  ContextService::singleton().add(
      ModeContext::name,
      std::move(std::unique_ptr<ModeContextCreator>(new ModeContextCreator(this))));

  m_textEditView->installEventFilter(this);
  m_textEditView->setCursorDrawer(std::unique_ptr<ViCursorDrawer>(new ViCursorDrawer(this)));

  m_mainWindow->statusBar()->addWidget(m_cmdLineEdit.get(), 1);
  m_cmdLineEdit->installEventFilter(this);

  connect(this, SIGNAL(modeChanged(Mode)), m_textEditView, SLOT(updateCursor()));
  connect(this, SIGNAL(enabled()), m_textEditView, SLOT(updateCursor()));
  connect(this, SIGNAL(disabled()), m_textEditView, SLOT(updateCursor()));
  connect(m_cmdLineEdit.get(), SIGNAL(returnPressed()), this, SLOT(cmdLineReturnPressed()));
  connect(m_cmdLineEdit.get(),
          SIGNAL(cursorPositionChanged(int, int)),
          this,
          SLOT(cmdLineCursorPositionChanged(int, int)));
  connect(m_cmdLineEdit.get(),
          SIGNAL(textChanged(QString)),
          this,
          SLOT(cmdLineTextChanged(const QString&)));

  m_mode = Mode::CMD;
  onModeChanged(m_mode);
  m_repeatCount = 0;

  KeymapService::singleton().load();

  m_isEnabled = true;

  emit enabled();
}

void ViEngine::disable() {
  CommandService::singleton().remove(ChangeModeCommand::name);
  ContextService::singleton().remove(ModeContext::name);

  m_textEditView->removeEventFilter(this);
  m_textEditView->resetCursorDrawer();

  m_mainWindow->statusBar()->removeWidget(m_cmdLineEdit.get());
  m_mainWindow->statusBar()->clearMessage();
  m_cmdLineEdit->removeEventFilter(this);

  disconnect(this, SIGNAL(modeChanged(Mode)), m_textEditView, SLOT(updateCursor()));
  disconnect(m_cmdLineEdit.get(), SIGNAL(returnPressed()), this, SLOT(cmdLineReturnPressed()));
  disconnect(m_cmdLineEdit.get(),
             SIGNAL(cursorPositionChanged(int, int)),
             this,
             SLOT(cmdLineCursorPositionChanged(int, int)));
  disconnect(m_cmdLineEdit.get(),
             SIGNAL(textChanged(QString)),
             this,
             SLOT(cmdLineTextChanged(const QString&)));

  KeymapService::singleton().load();

  m_isEnabled = false;
  emit disabled();
}

void ViEngine::setMode(Mode mode) {
  if (mode != m_mode) {
    if (m_mode == Mode::INSERT) {
      m_textEditView->moveCursor(QTextCursor::Left);
    }
    m_mode = mode;
    onModeChanged(mode);

    emit modeChanged(mode);
  }
}

void ViEngine::onModeChanged(Mode mode) {
  QString text;
  switch (mode) {
    case Mode::CMD:
      text = "CMD";
      break;
    case Mode::INSERT:
      text = "INSERT";
      break;
    case Mode::CMDLINE:
      m_cmdLineEdit->setText(":");
      m_cmdLineEdit->show();
      m_cmdLineEdit->setFocus(Qt::OtherFocusReason);
      return;
  }

  m_cmdLineEdit->hide();
  m_mainWindow->statusBar()->showMessage(text);
}

void ViEngine::cmdLineReturnPressed() {
  QString text = m_cmdLineEdit->text();
  if (!text.isEmpty() && text[0] == ':') {
    processExCommand(text.mid(1));
  }
}

void ViEngine::cmdLineCursorPositionChanged(int, int newPos) {
  if (newPos == 0) {
    m_cmdLineEdit->setCursorPosition(1);
  }
}

void ViEngine::cmdLineTextChanged(const QString& text) {
  if (text.isEmpty() || text[0] != ':') {
    setMode(Mode::CMD);
  }
}

bool ViEngine::eventFilter(QObject* obj, QEvent* event) {
  if (obj == m_textEditView && event->type() == QEvent::KeyPress && mode() == Mode::CMD) {
    cmdModeKeyPressEvent(static_cast<QKeyEvent*>(event));
    return true;
  }

  if (obj == m_cmdLineEdit.get() && event->type() == QEvent::KeyPress && mode() == Mode::CMDLINE) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      setMode(Mode::CMD);
      return true;
    }
  }

  return false;
}

void ViEngine::cmdModeKeyPressEvent(QKeyEvent* event) {
  QString text = event->text();
  if (text.isEmpty()) {
    return;
  }

  ushort ch = text[0].unicode();
  if ((ch == '0' && m_repeatCount != 0) || (ch >= '1' && ch <= '9')) {
    m_repeatCount = m_repeatCount * 10 + (ch - '0');
    return;
  }

  if (m_repeatCount > 0) {
    KeymapService::singleton().dispatch(event, m_repeatCount);
    m_repeatCount = 0;
  } else {
    KeymapService::singleton().dispatch(event);
  }
}

std::tuple<QRect, QColor> ViCursorDrawer::draw(const QRect& cursorRect) {
  QRect r = cursorRect;
  r.setWidth(cursorWidth());
  if (m_viEngine->mode() == Mode::CMD) {
    r = QRect(r.left(), r.top() + r.height() / 2, r.width(), r.height() / 2);
  }
  return std::make_tuple(r, QColor("red"));
}

void ViCursorDrawer::updateCursor(const TextEditView& TextEditView) {
  if (m_viEngine->mode() == Mode::CMD) {
    QTextCursor cur = TextEditView.textCursor();
    cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    QString text = cur.selectedText();
    QChar ch = text.isEmpty() ? QChar(' ') : text[0];
    int wd = TextEditView.fontMetrics().width(ch);
    if (!wd) {
      wd = TextEditView.fontMetrics().width(QChar(' '));
    }
    setCursorWidth(wd);
  } else {
    setCursorWidth(1);
  }
}
