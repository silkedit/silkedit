#include <memory>

#include "ViEngine.h"
#include "ViEditView.h"
#include "RubyEvaluator.h"
#include "CommandService.h"
#include "ContextService.h"
#include "KeymapService.h"
#include "ModeContext.h"
#include "commands/ChangeModeCommand.h"

ViEngine::ViEngine(ViEditView* viEditView, QObject* parent)
    : QObject(parent), m_mode(Mode::CMD), m_editor(viEditView), m_repeatCount(0) {
  m_editor->installEventFilter(this);
  m_editor->setCursorDrawer(std::unique_ptr<ViCursorDrawer>(new ViCursorDrawer(this)));

  std::unique_ptr<ChangeModeCommand> changeModeCmd(new ChangeModeCommand(this));
  CommandService::singleton().addCommand(std::move(changeModeCmd));

  ContextService::singleton().add(
      "mode", std::move(std::unique_ptr<ModeContextCreator>(new ModeContextCreator(this))));
}

void ViEngine::processExCommand(const QString&) {
  setMode(Mode::CMD);
}

void ViEngine::setMode(Mode mode) {
  if (mode != m_mode) {
    if (m_mode == Mode::INSERT) {
      m_editor->moveCursor(QTextCursor::Left);
    }
    m_mode = mode;
    emit modeChanged(mode);
  }
}

bool ViEngine::eventFilter(QObject* obj, QEvent* event) {
  if (obj == m_editor && event->type() == QEvent::KeyPress && mode() == Mode::CMD) {
    cmdModeKeyPressEvent(static_cast<QKeyEvent*>(event));
    return true;
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
