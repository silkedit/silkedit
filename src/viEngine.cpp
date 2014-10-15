#include <QtGui>
#include "viEngine.h"
#include "viEditView.h"

ViEngine::ViEngine(QObject *parent)
    : QObject(parent), m_mode(CMD), m_editor(nullptr) {}

ViEngine::~ViEngine() {}

void ViEngine::setEditor(ViEditView *editor) {
  m_editor = editor;
#if USE_EVENT_FILTER
  m_editor->installEventFilter(this);
#endif
}

void ViEngine::setMode(Mode mode) {
  if (mode != m_mode) {
    m_mode = mode;
    emit modeChanged(mode);
  }
}

#if 1
bool ViEngine::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_editor && event->type() == QEvent::KeyPress) {
    switch (mode()) {
    case CMD:
      cmdModeKeyPressEvent(static_cast<QKeyEvent *>(event));
      return true;
    case INSERT:
      return insertModeKeyPressEvent(static_cast<QKeyEvent *>(event));
    }
  }
  return false;
}
#else
bool ViEngine::processKeyPressEvent(QKeyEvent *event) {
  switch (mode()) {
  case CMD:
    return cmdModeKeyPressEvent(event);
  case INSERT:
    return insertModeKeyPressEvent(event);
  }
  return false;
}
#endif

bool ViEngine::cmdModeKeyPressEvent(QKeyEvent *event) {
  QString text = event->text();
  if (text.isEmpty()) return false;
  bool rc = true;
  ushort ch = text[0].unicode();
  if (ch == '0' && m_repeatCount != 0 || ch >= '1' && ch <= '9') {
    m_repeatCount = m_repeatCount * 10 + (ch - '0');
    return true;
  }

  switch (ch) {
  case 'i':
    setMode(INSERT);
    return true;
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
  default:
    rc = false;
    break;
  }

  m_repeatCount = 0;
  return rc;
}

bool ViEngine::insertModeKeyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    setMode(CMD);
    return true;
  }
  return false;
}
