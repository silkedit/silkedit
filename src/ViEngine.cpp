#include <memory>
#include <QStatusBar>

#include "SilkApp.h"
#include "Window.h"
#include "ViEngine.h"
#include "TextEditView.h"
#include "CommandManager.h"
#include "KeymapManager.h"
#include "ModeContext.h"
#include "commands/ChangeModeCommand.h"
#include "Context.h"

ViEngine::ViEngine(QObject* parent)
    : QObject(parent), m_mode(Mode::CMD), m_repeatCount(0), m_isEnabled(false) {
  ModeContext::singleton().init(this);
}

void ViEngine::enable() {
  qDebug("enabling ViEngine");

  std::unique_ptr<ChangeModeCommand> changeModeCmd(new ChangeModeCommand(this));
  CommandManager::add(std::move(changeModeCmd));

  KeyHandler::singleton().registerKeyEventFilter(this);
  if (auto view = SilkApp::activeEditView()) {
    view->setThinCursor(false);
  }

  m_mode = Mode::CMD;
  onModeChanged(m_mode);
  m_repeatCount = 0;

  KeymapManager::singleton().load();

  m_isEnabled = true;

  emit enabled();
}

void ViEngine::disable() {
  CommandManager::remove(ChangeModeCommand::name);

  KeyHandler::singleton().registerKeyEventFilter(this);
  if (auto view = SilkApp::activeEditView()) {
    view->setThinCursor(true);
  }

  foreach (Window* window, Window::windows()) { window->statusBar()->clearMessage(); }

  KeymapManager::singleton().load();

  m_isEnabled = false;
  emit disabled();
}

bool ViEngine::keyEventFilter(QKeyEvent* event) {
  if (event->type() == QEvent::KeyPress && mode() == Mode::CMD) {
    cmdModeKeyPressEvent(static_cast<QKeyEvent*>(event));
    return true;
  }

  if (event->type() == QEvent::KeyPress && mode() == Mode::CMDLINE) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      setMode(Mode::CMD);
      return true;
    }
  }

  return false;
}

void ViEngine::setMode(Mode mode) {
  if (mode != m_mode) {
    if (m_mode == Mode::INSERT && SilkApp::activeEditView()) {
      SilkApp::activeEditView()->moveCursor(QTextCursor::Left);
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
      return;
  }

  if (Window* window = SilkApp::activeWindow()) {
    Q_ASSERT(window->statusBar());
    window->statusBar()->showMessage(text);
  }

  updateCursor();
}

void ViEngine::updateCursor() {
  if (mode() == Mode::CMD) {
    if (auto view = SilkApp::activeEditView()) {
      view->setThinCursor(false);
    }
  } else {
    if (auto view = SilkApp::activeEditView()) {
      view->setThinCursor(true);
    }
  }
}

void ViEngine::cmdModeKeyPressEvent(QKeyEvent* event) {
  QString text = event->text();

  if (!text.isEmpty()) {
    ushort ch = text[0].unicode();
    if ((ch == '0' && m_repeatCount != 0) || (ch >= '1' && ch <= '9')) {
      m_repeatCount = m_repeatCount * 10 + (ch - '0');
      return;
    }
  }

  if (m_repeatCount > 0) {
    KeymapManager::singleton().dispatch(event, m_repeatCount);
    m_repeatCount = 0;
  } else {
    KeymapManager::singleton().dispatch(event);
  }
}
