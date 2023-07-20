QT += widgets

TEMPLATE = lib
DEFINES += TASKSHEETMANAGER_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Client/ctablemodel.cpp \
    ../Client/mycombobox.cpp \
    ../Client/mymodel.cpp \
    ../Client/mytableview.cpp \
    ../Client/qjsoncmd.cpp \
    ../Client/tabwigetbase.cpp \
    clientmanagerdlg.cpp \
    clineteditor.cpp \
    implementingstandardselectdlg.cpp \
    methodselectdlg.cpp \
    tasksheeteditor.cpp \
    tasksheetui.cpp \
    testinfoeditor.cpp

HEADERS += \
    ../Client/ctablemodel.h \
    ../Client/mycombobox.h \
    ../Client/mymodel.h \
    ../Client/mytableview.h \
    ../Client/qjsoncmd.h \
    ../Client/tabwigetbase.h \
    TaskSheetManager_global.h \
    clientmanagerdlg.h \
    clineteditor.h \
    implementingstandardselectdlg.h \
    methodselectdlg.h \
    tasksheeteditor.h \
    tasksheetui.h \
    testinfoeditor.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    clientmanagerdlg.ui \
    clineteditor.ui \
    implementingstandardselectdlg.ui \
    methodselectdlg.ui \
    tasksheeteditor.ui \
    tasksheetui.ui \
    testinfoeditor.ui


DESTDIR = "../../lib"
CONFIG(debug,debug|release){
    DLLDESTDIR = "../../debug"
    TARGET = TaskSheetManagerd
}else{
    DLLDESTDIR = "../../bin"
    TARGET = TaskSheetManager
}
INCLUDEPATH += $$PWD/../Client
