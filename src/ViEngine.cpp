#include <memory>
#include <QtGui>
#include "ViEngine.h"
#include "ViEditView.h"
#include "RubyEvaluator.h"
#include "CommandService.h"
#include "KeymapService.h"
#include "commands/ChangeModeCommand.h"

ViEngine::ViEngine(QObject* parent) : QObject(parent), m_mode(Mode::CMD), m_editor(nullptr) {
  std::unique_ptr<ChangeModeCommand> cmd(new ChangeModeCommand(this));
  CommandService::singleton().addCommand(std::move(cmd));
}

ViEngine::~ViEngine() {
}

void ViEngine::setEditor(ViEditView* editor) {
  m_editor = editor;
#if USE_EVENT_FILTER
  m_editor->installEventFilter(this);
#endif
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

#if 1
bool ViEngine::eventFilter(QObject* obj, QEvent* event) {
  if (obj == m_editor && event->type() == QEvent::KeyPress) {
    // TODO: KeymapService must dispatch!
    switch (mode()) {
      case Mode::CMD:
        cmdModeKeyPressEvent(static_cast<QKeyEvent*>(event));
        return true;
      case Mode::INSERT:
        return insertModeKeyPressEvent(static_cast<QKeyEvent*>(event));
      default:
        // TODO: add logging
        break;
    }
  }
  return false;
}
#else
bool ViEngine::processKeyPressEvent(QKeyEvent* event) {
  switch (mode()) {
    case CMD:
      return cmdModeKeyPressEvent(event);
    case INSERT:
      return insertModeKeyPressEvent(event);
  }
  return false;
}
#endif

bool ViEngine::cmdModeKeyPressEvent(QKeyEvent* event) {
  QString text = event->text();
  if (text.isEmpty())
    return false;
  bool rc = true;
  ushort ch = text[0].unicode();
  if ((ch == '0' && m_repeatCount != 0) || (ch >= '1' && ch <= '9')) {
    m_repeatCount = m_repeatCount * 10 + (ch - '0');
    return true;
  }

  bool isHandled = KeymapService::singleton().dispatch(*event);
  if (!isHandled) {
    switch (ch) {
      case 'h':
        m_editor->moveCursor(QTextCursor::Left, repeatCount());
        break;
      case ' ':
      case 'l':
        m_editor->moveCursor(QTextCursor::Right, repeatCount());
        break;
      case 'k':
        m_editor->moveCursor(QTextCursor::Up, repeatCount());
        break;
      case 'j':
        m_editor->moveCursor(QTextCursor::Down, repeatCount());
        break;
      case ':':
        setMode(Mode::CMDLINE);
        break;
      case 'x':
        m_editor->doDelete(repeatCount());
        break;
      case 'X':
        m_editor->doDelete(-repeatCount());
        break;
      case 'u':
        m_editor->doUndo(repeatCount());
        break;
      case 'U':
        m_editor->doRedo(repeatCount());
        break;
      case '0':
        m_editor->moveCursor(QTextCursor::StartOfBlock);
        break;
      case '^':
        m_editor->moveCursor(ViMoveOperation::FirstNonBlankChar);
        break;
      case '$':
        m_editor->moveCursor(ViMoveOperation::LastChar);
        break;
      case '\r':  // Enter
      case '+':
        m_editor->moveCursor(ViMoveOperation::NextLine);
        break;
      case '-':
        m_editor->moveCursor(ViMoveOperation::PrevLine);
        break;
      case 'r': {
        RubyEvaluator& evaluator = RubyEvaluator::singleton();
        evaluator.eval(m_editor->toPlainText());
        break;
      }
      default:
        rc = false;
        break;
    }
  }

  m_repeatCount = 0;
  return rc;
}

bool ViEngine::insertModeKeyPressEvent(QKeyEvent* event) {
  bool isHandled = KeymapService::singleton().dispatch(*event);
  if (isHandled) {
    return true;
  }

  if (event->key() == Qt::Key_Escape) {
    if (mode() == Mode::INSERT) {
      m_editor->moveCursor(QTextCursor::Left);
    }
    setMode(Mode::CMD);
    return true;
  }
  return false;
}
