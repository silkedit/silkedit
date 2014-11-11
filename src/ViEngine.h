#pragma once

#include <QObject>

#include "vi.h"
#include "ICursorDrawer.h"

class QKeyEvent;
class ViEditView;

class ViEngine : public QObject {
  Q_OBJECT

 public:
  ViEngine(ViEditView* viEditView, QObject* parent = nullptr);
  ~ViEngine() = default;

  Mode mode() const { return m_mode; }

  void setMode(Mode mode);

  void processExCommand(const QString& text);

signals:
  void modeChanged(Mode);

 protected:
  bool eventFilter(QObject*, QEvent*);
  void cmdModeKeyPressEvent(QKeyEvent*);

 private:
  Mode m_mode;
  ViEditView* m_editor;
  int m_repeatCount;
};

class ViCursorDrawer : public ICursorDrawer {
  DISABLE_COPY(ViCursorDrawer)
 public:
  ViCursorDrawer(ViEngine* viEngine) : m_viEngine(viEngine) {}
  ~ViCursorDrawer() = default;
  DEFAULT_MOVE(ViCursorDrawer)

  std::tuple<QRect, QColor> draw(const QRect& cursorRect) override;

 private:
  ViEngine* m_viEngine;
};
