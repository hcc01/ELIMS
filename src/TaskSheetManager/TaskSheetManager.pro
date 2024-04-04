QT += widgets axcontainer sql

TEMPLATE = lib
DEFINES += TASKSHEETMANAGER_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Client/ctablemodel.cpp \
    ../Client/dbmater.cpp \
    ../Client/itemsselectdlg.cpp \
    ../Client/mycombobox.cpp \
    ../Client/mymodel.cpp \
    ../Client/mytableview.cpp \
    ../Client/qexcel.cpp \
    ../Client/qjsoncmd.cpp \
    ../Client/tabwigetbase.cpp \
    clientmanagerdlg.cpp \
    clineteditor.cpp \
    contractreviewdlg.cpp \
    doscheduledlg.cpp \
    implementingstandardselectdlg.cpp \
    methodselectdlg.cpp \
    reportmanagerui.cpp \
    samplecirculationui.cpp \
    samplegroupingdlg.cpp \
    samplingscheduleui.cpp \
    tasksheeteditor.cpp \
    tasksheetui.cpp \
    testinfoeditor.cpp \
    testmanager.cpp \
    workhoursatistics.cpp

HEADERS += \
    ../Client/ctablemodel.h \
    ../Client/dbmater.h \
    ../Client/global.h \
    ../Client/itemsselectdlg.h \
    ../Client/mycombobox.h \
    ../Client/mymodel.h \
    ../Client/mytableview.h \
    ../Client/qexcel.h \
    ../Client/qjsoncmd.h \
    ../Client/tabwigetbase.h \
    TaskSheetManager_global.h \
    clientmanagerdlg.h \
    clineteditor.h \
    contractreviewdlg.h \
    doscheduledlg.h \
    implementingstandardselectdlg.h \
    methodselectdlg.h \
    reportmanagerui.h \
    samplecirculationui.h \
    samplegroupingdlg.h \
    samplingscheduleui.h \
    tasksheeteditor.h \
    tasksheetui.h \
    testinfoeditor.h \
    testmanager.h \
    workhoursatistics.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    ../Client/itemsselectdlg.ui \
    clientmanagerdlg.ui \
    clineteditor.ui \
    contractreviewdlg.ui \
    doscheduledlg.ui \
    implementingstandardselectdlg.ui \
    methodselectdlg.ui \
    reportmanagerui.ui \
    samplecirculationui.ui \
    samplegroupingdlg.ui \
    samplingscheduleui.ui \
    tasksheeteditor.ui \
    tasksheetui.ui \
    testinfoeditor.ui \
    testmanager.ui \
    workhoursatistics.ui


DESTDIR = "../../lib"
CONFIG(debug,debug|release){
    DLLDESTDIR = "../../debug"
    TARGET = TaskSheetManagerd
}else{
    DLLDESTDIR = "../../bin"
    TARGET = TaskSheetManager
}
INCLUDEPATH += $$PWD/../Client

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lsqlpagecontroleui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lsqlpagecontroleuid
else:unix: LIBS += -L$$PWD/../../lib/ -lsqlpagecontroleui

INCLUDEPATH += $$PWD/../sqlPageControleUI
DEPENDPATH += $$PWD/../sqlPageControleUI

include(../qzxing/QZXing.pri)
include(../QXlsx/QXlsx.pri)

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/ -lExcelOperator
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/ -lExcelOperatord
else:unix: LIBS += -L$$PWD/../../lib/ -lExcelOperator

INCLUDEPATH += $$PWD/../ExcelOperator
DEPENDPATH += $$PWD/../ExcelOperator
