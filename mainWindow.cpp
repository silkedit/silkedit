#include "mainWindow.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
    m_editor = new ViEditView;
    m_editor->setWindowTitle(QObject::tr("Code Editor Example"));
    setCentralWidget(m_editor);
}

MainWindow::~MainWindow()
{

}
