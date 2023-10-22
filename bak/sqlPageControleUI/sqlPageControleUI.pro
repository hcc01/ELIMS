QT += widgets

TEMPLATE = lib
DEFINES += SQLPAGECONTROLEUI_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    sqlpagecontroleui.cpp\
    ../Client/qjsoncmd.cpp

HEADERS += \
    sqlPageControleUI_global.h \
    ../Client/qjsoncmd.cpp \
    sqlpagecontroleui.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    pageControler.ui

DESTDIR = "../../lib"
CONFIG(debug,debug|release){
    DLLDESTDIR = "../../debug"
    TARGET = sqlpagecontroleuid
}else{
    DLLDESTDIR = "../../bin"
    TARGET = sqlpagecontroleui
}
