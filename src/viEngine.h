#pragma once

#include <QObject>
#include "vi.h"

class QKeyEvent;
class ViEditView;

class ViEngine : public QObject {
  Q_OBJECT

public:
  ViEngine(QObject *parent = 0);
  ~ViEngine();

  Mode mode() const { return m_mode; }

  void setMode(Mode mode);
  void setEditor(ViEditView *editor);

#if !USE_EVENT_FILTER
  bool processKeyPressEvent(QKeyEvent *event);
#endif

signals:
  void modeChanged(Mode);

protected:
  bool eventFilter(QObject *, QEvent *);
  bool cmdModeKeyPressEvent(QKeyEvent *);
  bool insertModeKeyPressEvent(QKeyEvent *);
  int repeatCount() const { return !m_repeatCount ? 1 : m_repeatCount; }

private:
  Mode m_mode;
  ViEditView *m_editor;
  int m_repeatCount;
};
