#pragma once

#include <QString>
#include <QDebug>

#include "../macros.h"

class ICommand {
  DISABLE_COPY(ICommand)
public:
  ICommand(QString name): m_name(name) {}
  ICommand(ICommand&&) = default;
  virtual ~ICommand() = default;

  ICommand& operator=(ICommand&&) = default;

  inline void run() {
    qDebug() << "Start command: " << m_name;
    doRun();
    qDebug() << "End command: " << m_name;
  }

  inline QString name() { return m_name; }

private:
  virtual void doRun() = 0;

  QString m_name;
};
