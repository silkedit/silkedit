#pragma once

#include <memory>
#include <QObject>
#include <QLineEdit>

#include "vi.h"
#include "IKeyEventFilter.h"

class QKeyEvent;
class TextEditView;
class MainWindow;
class LayoutView;

class ViEngine : public QObject, public IKeyEventFilter {
  Q_OBJECT

 public:
  ViEngine(MainWindow* mainWindow, QObject* parent = nullptr);
  ~ViEngine() = default;

  Mode mode() const { return m_mode; }
  void setMode(Mode mode);

  void processExCommand(const QString& text);
  void enable();
  void disable();
  bool isEnabled() { return m_isEnabled; }
  bool keyEventFilter(QKeyEvent* event) override;

signals:
  void modeChanged(Mode);
  void enabled();
  void disabled();

 protected:
  void cmdModeKeyPressEvent(QKeyEvent*);

 private:
  Mode m_mode;
  MainWindow* m_mainWindow;
  int m_repeatCount;
  std::unique_ptr<QLineEdit> m_cmdLineEdit;
  bool m_isEnabled;

  void onModeChanged(Mode);

 private slots:
  void cmdLineReturnPressed();
  void cmdLineCursorPositionChanged(int, int);
  void cmdLineTextChanged(const QString& text);
  void updateCursor();
};
