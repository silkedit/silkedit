#include <memory>
#include <QStatusBar>

#include "API.h"
#include "MainWindow.h"
#include "ViEngine.h"
#include "TextEditView.h"
#include "CommandService.h"
#include "ContextService.h"
#include "KeymapService.h"
#include "ModeContext.h"
#include "commands/ChangeModeCommand.h"

ViEngine::ViEngine(QObject* parent)
    : QObject(parent), m_mode(Mode::CMD), m_repeatCount(0), m_isEnabled(false) {}

void ViEngine::enable() {
  qDebug("enabling ViEngine");

  std::unique_ptr<ChangeModeCommand> changeModeCmd(new ChangeModeCommand(this));
  CommandService::add(std::move(changeModeCmd));

  ContextService::singleton().add(
      ModeContext::name,
      std::move(std::unique_ptr<ModeContextCreator>(new ModeContextCreator(this))));

  KeyHandler::singleton().registerKeyEventFilter(this);
  if (auto view = API::activeEditView()) {
    view->setThinCursor(false);
  }

  m_mode = Mode::CMD;
  onModeChanged(m_mode);
  m_repeatCount = 0;

  KeymapService::singleton().load();

  m_isEnabled = true;

  emit enabled();
}

void ViEngine::disable() {
  CommandService::remove(ChangeModeCommand::name);
  ContextService::singleton().remove(ModeContext::name);

  KeyHandler::singleton().registerKeyEventFilter(this);
  if (auto view = API::activeEditView()) {
    view->setThinCursor(true);
  }

  foreach(MainWindow * window, API::windows()) { window->statusBar()->clearMessage(); }

  KeymapService::singleton().load();

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
    if (m_mode == Mode::INSERT && API::activeEditView()) {
      API::activeEditView()->moveCursor(QTextCursor::Left);
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

  if (MainWindow* window = API::activeWindow()) {
    Q_ASSERT(window->statusBar());
    window->statusBar()->showMessage(text);
  }

  updateCursor();
}

void ViEngine::updateCursor() {
  if (mode() == Mode::CMD) {
    if (auto view = API::activeEditView()) {
      view->setThinCursor(false);
    }
  } else {
    if (auto view = API::activeEditView()) {
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
    KeymapService::singleton().dispatch(event, m_repeatCount);
    m_repeatCount = 0;
  } else {
    KeymapService::singleton().dispatch(event);
  }
}
