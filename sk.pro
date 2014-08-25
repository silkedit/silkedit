QT += widgets

HEADERS     = sk.h
SOURCES     = main.cpp \
              sk.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/codeeditor
INSTALLS += target
