QT += widgets

HEADERS     = \
    mainWindow.h \
    viEditView.h
SOURCES     = main.cpp \
    mainWindow.cpp \
    viEditView.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/codeeditor
INSTALLS += target

CONFIG+=c++11
