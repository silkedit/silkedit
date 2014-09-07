#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "viEditView.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
    ~MainWindow();

private:
    MainWindow(const MainWindow&);
    MainWindow& operator=(const MainWindow&);

    ViEditView *m_editor;
};

#endif // MAINWINDOW_H
