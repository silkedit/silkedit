#pragma once

#include <memory>
#include <QObject>
#include <QLineEdit>

#include "vi.h"
#include "IKeyEventFilter.h"

class QKeyEvent;
class TextEditView;

class ViEngine : public QObject, public IKeyEventFilter {
  Q_OBJECT

 public:
  ViEngine(QObject* parent = nullptr);
  ~ViEngine() = default;

  Mode mode() const { return m_mode; }
  void setMode(Mode mode);

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
  int m_repeatCount;
  bool m_isEnabled;

  void onModeChanged(Mode);

 private slots:
  void updateCursor();
};
