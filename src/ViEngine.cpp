#include <memory>
#include <QtGui>

#include "ViEngine.h"
#include "ViEditView.h"
#include "RubyEvaluator.h"
#include "CommandService.h"
#include "KeymapService.h"
#include "commands/ChangeModeCommand.h"

ViEngine::ViEngine(ViEditView* viEditView, QObject* parent)
    : QObject(parent), m_mode(Mode::CMD), m_editor(viEditView) {
  m_editor->installEventFilter(this);

  std::unique_ptr<ChangeModeCommand> changeModeCmd(new ChangeModeCommand(this));
  CommandService::singleton().addCommand(std::move(changeModeCmd));
}

void ViEngine::processExCommand(const QString& text) {
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
  if (obj == m_editor && event->type() == QEvent::KeyPress) {
    switch (mode()) {
      case Mode::CMD:
        cmdModeKeyPressEvent(static_cast<QKeyEvent*>(event));
        return true;
      case Mode::INSERT:
        return KeymapService::singleton().dispatch(static_cast<QKeyEvent*>(event));
      default:
        // TODO: add logging
        break;
    }
  }
  return false;
}

bool ViEngine::cmdModeKeyPressEvent(QKeyEvent* event) {
  QString text = event->text();
  if (text.isEmpty())
    return false;
  ushort ch = text[0].unicode();
  if ((ch == '0' && m_repeatCount != 0) || (ch >= '1' && ch <= '9')) {
    m_repeatCount = m_repeatCount * 10 + (ch - '0');
    return true;
  }

  if (m_repeatCount > 0) {
    bool isHandled = KeymapService::singleton().dispatch(event, m_repeatCount);
    m_repeatCount = 0;
    return isHandled;
  } else {
    return KeymapService::singleton().dispatch(event);
  }
}
