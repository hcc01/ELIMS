QT += widgets

TEMPLATE = lib
DEFINES += WORKFLOWUI_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
../Client/ctablemodel.cpp \
../Client/qjsoncmd.cpp \
../Client/tabwigetbase.cpp \
    workflowui.cpp

HEADERS += \
../Client/ctablemodel.h \
../Client/qjsoncmd.h \
../Client/tabwigetbase.h \
    WorkFlowUI_global.h \
    workflowui.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

DESTDIR = "../../lib"
CONFIG(debug,debug|release){
    DLLDESTDIR = "../../debug"
    TARGET = WorkFlowManagerUId
}else{
    DLLDESTDIR = "../../bin"
    TARGET = WorkFlowManagerUI
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lsqlpagecontroleui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lsqlpagecontroleuid
else:unix: LIBS += -L$$PWD/../../lib/ -lsqlpagecontroleui

INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include
