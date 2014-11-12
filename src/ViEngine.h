#pragma once

#include <memory>
#include <QObject>
#include <QLineEdit>

#include "vi.h"
#include "ICursorDrawer.h"

class QKeyEvent;
class TextEditView;
class MainWindow;

class ViEngine : public QObject {
  Q_OBJECT

 public:
  ViEngine(TextEditView* textEditView, MainWindow* mainWindow, QObject* parent = nullptr);
  ~ViEngine() = default;

  Mode mode() const { return m_mode; }
  void setMode(Mode mode);

  void processExCommand(const QString& text);
  void enable();
  void disable();
  bool isEnabled() { return m_isEnabled; }

signals:
  void modeChanged(Mode);
  void enabled();
  void disabled();

 protected:
  bool eventFilter(QObject*, QEvent*);
  void cmdModeKeyPressEvent(QKeyEvent*);

 private:
  Mode m_mode;
  TextEditView* m_textEditView;
  MainWindow* m_mainWindow;
  int m_repeatCount;
  std::unique_ptr<QLineEdit> m_cmdLineEdit;
  bool m_isEnabled;

  void onModeChanged(Mode);

 private slots:
  void cmdLineReturnPressed();
  void cmdLineCursorPositionChanged(int, int);
  void cmdLineTextChanged(const QString& text);
};

class ViCursorDrawer : public ICursorDrawer {
  DISABLE_COPY(ViCursorDrawer)
 public:
  ViCursorDrawer(ViEngine* viEngine) : m_viEngine(viEngine) {}
  ~ViCursorDrawer() = default;
  DEFAULT_MOVE(ViCursorDrawer)

  std::tuple<QRect, QColor> draw(const QRect& cursorRect) override;
  void updateCursor(const TextEditView& TextEditView) override;

 private:
  ViEngine* m_viEngine;
};
